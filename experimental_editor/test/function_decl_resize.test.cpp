#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/actors/function_decl_actor.hpp"
#include "editor/actors/port_actor.hpp"
#include "editor/actors/scene.hpp"
#include "editor/components/resize_handle.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/layer.hpp"
#include "editor/core/viewport.hpp"
#include "editor/transaction/resize.hpp"
#include "editor/transaction/transaction.hpp"
#include "fixture_loader.hpp"
#include "recording_renderer.hpp"

// The XY grip on a function frame. single_empty_function.fl declares foo at
// x=10 y=10 w=100 h=100; unitPx 5 -> frame bounds {50,50,500,500}, header grip
// {530,55,15,15} and corner grip {535,535,15,15}. The two do not overlap on a
// frame this size. onDrag takes a *world-pixel* delta and truncates to whole
// units, so n * unitPx is exactly n units.

namespace {

  using fluir::FlowGraphLocation;
  using fluir::editor::EditorContext;
  using fluir::editor::FunctionDeclActor;
  using fluir::editor::GraphScene;
  using fluir::editor::Rect;
  using fluir::editor::Transaction;
  using fluir::editor::Vec2;
  using fluir::editor::XYResizeHandle;
  using testutil::hasFill;
  using testutil::Loaded;
  using testutil::loadFixture;
  using testutil::RecordingRenderer;

  constexpr Vec2 kCornerGrip{542.5, 542.5};  // centre of {535,535,15,15}
  constexpr Vec2 kHeaderGrip{537.5, 62.5};   // centre of {530,55,15,15}

  /** A scene over `fixture` plus a context whose commits land in `edits`. */
  struct Harness {
    Loaded loaded;
    EditorContext ctx;
    GraphScene scene;
    std::vector<std::unique_ptr<Transaction>> edits;

    explicit Harness(const std::string& fixture) : loaded(loadFixture(fixture)) {
      ctx.commit = [this](std::unique_ptr<Transaction> edit) {
        edits.push_back(std::move(edit));
        return true;
      };
    }

    /** Builds the scene; returns the frame actor for function id 1. */
    FunctionDeclActor* build() {
      if (!loaded.result.tree.has_value()) {
        return nullptr;
      }
      scene.build(ctx, *loaded.result.tree);
      return dynamic_cast<FunctionDeclActor*>(scene.find(1));
    }
  };

  // Draws `scene` through one Layer with an identity viewport, as ModulePage does.
  void drawScene(const GraphScene& scene, const EditorContext& ctx, RecordingRenderer& r) {
    fluir::editor::Layer layer;
    layer.setRoot(scene.root());
    layer.draw(r, ctx, Rect{0, 0, r.outputSize().x, r.outputSize().y});
  }

}  // namespace

TEST(FunctionDeclResize, OnDragStartClaimsOnlyOnTheCornerGrip) {
  Harness h("read/single_empty_function.fl");
  FunctionDeclActor* frame = h.build();
  ASSERT_NE(frame, nullptr);

  EXPECT_TRUE(frame->gestures()->press(h.ctx, kCornerGrip));
  EXPECT_FALSE(frame->gestures()->press(h.ctx, Vec2{300, 300}));    // inside the frame, off both grips
  EXPECT_FALSE(frame->gestures()->press(h.ctx, Vec2{1000, 1000}));  // off the frame entirely
}

TEST(FunctionDeclResize, TheHeaderGripStillMovesTheFrame) {
  Harness h("read/single_empty_function.fl");
  FunctionDeclActor* frame = h.build();
  ASSERT_NE(frame, nullptr);

  ASSERT_TRUE(frame->gestures()->press(h.ctx, kHeaderGrip));
  frame->gestures()->drag(h.ctx, Vec2{10, 10});
  frame->gestures()->release(h.ctx);

  ASSERT_EQ(h.edits.size(), 1u);
  ASSERT_TRUE(h.edits[0]->execute(h.scene));
  const FlowGraphLocation* loc = frame->location();
  ASSERT_NE(loc, nullptr);
  EXPECT_EQ(loc->x, 12);
  EXPECT_EQ(loc->y, 12);
  EXPECT_EQ(loc->width, 100) << "a header drag never resizes";
  EXPECT_EQ(loc->height, 100);
}

TEST(FunctionDeclResize, AResizeLeavesTheModelUntouchedUntilRelease) {
  Harness h("read/single_empty_function.fl");
  FunctionDeclActor* frame = h.build();
  ASSERT_NE(frame, nullptr);

  ASSERT_TRUE(frame->gestures()->press(h.ctx, kCornerGrip));
  frame->gestures()->drag(h.ctx, Vec2{10, 10});  // 2 grid units on each axis
  h.scene.layout(h.ctx);

  EXPECT_EQ(frame->bounds(), (Rect{50, 50, 510, 510})) << "the preview grows on screen";
  EXPECT_EQ(frame->location()->width, 100) << "the model only grows on commit";
  EXPECT_EQ(frame->location()->height, 100);
}

TEST(FunctionDeclResize, ReleaseRaisesOneResizeForTheWholeGesture) {
  Harness h("read/single_empty_function.fl");
  FunctionDeclActor* frame = h.build();
  ASSERT_NE(frame, nullptr);

  ASSERT_TRUE(frame->gestures()->press(h.ctx, kCornerGrip));
  frame->gestures()->drag(h.ctx, Vec2{10, 0});
  frame->gestures()->drag(h.ctx, Vec2{5, 5});
  frame->gestures()->release(h.ctx);

  ASSERT_EQ(h.edits.size(), 1u);
  ASSERT_TRUE(h.edits[0]->execute(h.scene));
  EXPECT_EQ(frame->location()->width, 103);
  EXPECT_EQ(frame->location()->height, 101);
}

TEST(FunctionDeclResize, AVerticalDragResizesHeightOnly) {
  Harness h("read/single_empty_function.fl");
  FunctionDeclActor* frame = h.build();
  ASSERT_NE(frame, nullptr);

  ASSERT_TRUE(frame->gestures()->press(h.ctx, kCornerGrip));
  frame->gestures()->drag(h.ctx, Vec2{0, 10});
  frame->gestures()->release(h.ctx);

  ASSERT_EQ(h.edits.size(), 1u);
  ASSERT_TRUE(h.edits[0]->execute(h.scene));
  EXPECT_EQ(frame->location()->height, 102);
  EXPECT_EQ(frame->location()->width, 100);
}

TEST(FunctionDeclResize, AReleaseThatResizedNothingRaisesNoEdit) {
  Harness h("read/single_empty_function.fl");
  FunctionDeclActor* frame = h.build();
  ASSERT_NE(frame, nullptr);

  ASSERT_TRUE(frame->gestures()->press(h.ctx, kCornerGrip));
  frame->gestures()->drag(h.ctx, Vec2{3, 3});  // sub-grid: nothing resized
  frame->gestures()->release(h.ctx);

  EXPECT_TRUE(h.edits.empty());
}

TEST(FunctionDeclResize, CancelDropsThePreview) {
  Harness h("read/single_empty_function.fl");
  FunctionDeclActor* frame = h.build();
  ASSERT_NE(frame, nullptr);

  ASSERT_TRUE(frame->gestures()->press(h.ctx, kCornerGrip));
  frame->gestures()->drag(h.ctx, Vec2{10, 10});
  frame->gestures()->cancel();
  h.scene.layout(h.ctx);

  EXPECT_EQ(frame->bounds(), (Rect{50, 50, 500, 500}));
  EXPECT_TRUE(h.edits.empty());
}

TEST(FunctionDeclResize, PreviewClampsToTheMinimumSize) {
  Harness h("read/single_empty_function.fl");
  FunctionDeclActor* frame = h.build();
  ASSERT_NE(frame, nullptr);

  ASSERT_TRUE(frame->gestures()->press(h.ctx, kCornerGrip));
  frame->gestures()->drag(h.ctx, Vec2{-1000, -1000});  // -200 grid units, far past zero
  h.scene.layout(h.ctx);

  EXPECT_GT(frame->bounds().w, 0);
  EXPECT_GT(frame->bounds().h, 0);

  frame->gestures()->release(h.ctx);
  ASSERT_EQ(h.edits.size(), 1u);
  ASSERT_TRUE(h.edits[0]->execute(h.scene));
  EXPECT_GT(frame->location()->width, 0);
  EXPECT_GT(frame->location()->height, 0);
}

TEST(FunctionDeclResize, DrawRendersTheCornerGripFromItsRect) {
  Harness h("read/single_empty_function.fl");
  FunctionDeclActor* frame = h.build();
  ASSERT_NE(frame, nullptr);

  // frameRect = localRect(loc, unitPx) = {50,50,500,500}; the grip is grid units
  // {loc.width - SIZE, loc.height - SIZE, SIZE, SIZE} = {97,97,3,3}
  // -> world {50 + 97*5, 50 + 97*5, 3*5, 3*5} = {535,535,15,15}.
  const double u = h.ctx.layout.unitPx;
  const FlowGraphLocation* loc = frame->location();
  ASSERT_NE(loc, nullptr);
  const Rect frameRect{loc->x * u, loc->y * u, loc->width * u, loc->height * u};
  const Rect gr = XYResizeHandle{{0, 0}}.rect(*loc);
  const Rect grip{frameRect.x + gr.x * u, frameRect.y + gr.y * u, gr.w * u, gr.h * u};

  EXPECT_EQ(grip, (Rect{535, 535, 15, 15}));

  RecordingRenderer r;
  drawScene(h.scene, h.ctx, r);
  EXPECT_TRUE(hasFill(r.calls, grip));
}

// Regression: layout must push the *live preview* width to the return rail, not
// the committed one, or the rail lags a whole gesture behind the frame edge.
TEST(FunctionDeclResize, TheReturnRailTracksTheLivePreview) {
  Harness h("read/function_with_output_only.fl");
  FunctionDeclActor* frame = h.build();
  ASSERT_NE(frame, nullptr);
  const fluir::editor::PortActor* rail = frame->port(4);
  ASSERT_NE(rail, nullptr);
  const Rect atRest = rail->worldBounds();

  ASSERT_TRUE(frame->gestures()->press(h.ctx, kCornerGrip));
  frame->gestures()->drag(h.ctx, Vec2{2 * h.ctx.layout.unitPx, 0});
  h.scene.layout(h.ctx);

  EXPECT_TRUE(h.edits.empty()) << "still mid-gesture";
  EXPECT_EQ(rail->worldBounds().x, atRest.x + 2 * h.ctx.layout.unitPx);
  EXPECT_EQ(rail->worldBounds().y, atRest.y);
}
