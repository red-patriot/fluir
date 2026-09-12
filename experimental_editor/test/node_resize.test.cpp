#include <memory>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
#include "editor/actors/node_actors.hpp"
#include "editor/actors/scene.hpp"
#include "editor/components/resize_handle.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"
#include "editor/transaction/transaction.hpp"
#include "recording_renderer.hpp"

namespace {

  using fluir::FlowGraphLocation;
  using fluir::ID;
  using fluir::Operator;
  using fluir::editor::BinaryActor;
  using fluir::editor::EditorContext;
  using fluir::editor::GraphScene;
  using fluir::editor::Rect;
  using fluir::editor::Subview;
  using fluir::editor::Transaction;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::hasFill;
  using testutil::RecordingRenderer;

  constexpr ID kFunctionId = 100;

  // width/height = 5 units, unitPx = 5 (EditorContext default) -> a 25x25
  // local-space node rect, so the resize bar is the world rect {20, 0, 5, 25}.
  const FlowGraphLocation kNodeLoc{.x = 0, .y = 0, .z = 0, .width = 5, .height = 5};

  fluir::pt::Binary makeBinary(FlowGraphLocation loc = kNodeLoc) {
    fluir::pt::Binary binary;
    binary.id = 1;
    binary.location = loc;
    binary.lhs = 2;
    binary.rhs = 3;
    binary.op = Operator::PLUS;
    return binary;
  }

  // Identity Viewport + a root Subview at world origin, so
  // `body.toScreen(local) == local`; returns every recorded draw call.
  std::vector<testutil::DrawCall> recordDraw(BinaryActor& actor, const EditorContext& ctx) {
    actor.layout(ctx);
    RecordingRenderer renderer;
    const Viewport viewport;
    {
      const Subview body{viewport, Rect{0, 0, 1000, 1000}, renderer};
      actor.draw(body, ctx);
    }
    return renderer.calls;
  }

  // A scene holding exactly the node the bare actor stands for, so an edit the
  // actor raises can be applied and read back.
  fluir::pt::ParseTree makeTree(FlowGraphLocation loc = kNodeLoc) {
    fluir::pt::Constant constant;
    constant.id = 1;
    constant.location = loc;
    constant.value = fluir::literals_types::I32{0};

    fluir::pt::FunctionDecl fn;
    fn.id = kFunctionId;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    fn.body.nodes.emplace(constant.id, constant);

    fluir::pt::ParseTree tree;
    tree.declarations.emplace(fn.id, fluir::pt::Declaration{fn});
    return tree;
  }

}  // namespace

TEST(NodeResize, OnDragStartClaimsOnlyOnTheRightEdgeBar) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  const EditorContext ctx;

  EXPECT_TRUE(actor.onDragStart(ctx, Vec2{22, 22}));     // on the bar, clear of the drag grip
  EXPECT_FALSE(actor.onDragStart(ctx, Vec2{12, 22}));    // inside the node, off both grips
  EXPECT_FALSE(actor.onDragStart(ctx, Vec2{100, 100}));  // off the node entirely
}

TEST(NodeResize, TheDragGripWinsWhereTheGripsOverlap) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = [&](std::unique_ptr<Transaction> edit) {
    edits.push_back(std::move(edit));
    return true;
  };

  ASSERT_TRUE(actor.onDragStart(ctx, Vec2{19, 10}));  // the drag grip's rightmost pixels
  actor.onDrag(ctx, Vec2{}, Vec2{10, 0});
  actor.onDragEnd(ctx, Vec2{});

  ASSERT_EQ(edits.size(), 1u);

  GraphScene scene;
  scene.build(ctx, makeTree());
  ASSERT_TRUE(edits[0]->execute(scene));
  const fluir::FlowGraphLocation* loc = scene.find(kFunctionId, 1)->location();
  ASSERT_NE(loc, nullptr);
  EXPECT_EQ(loc->x, 2) << "the gesture moved the node";
  EXPECT_EQ(loc->width, 5) << "and did not resize it";
}

TEST(NodeResize, AResizeLeavesTheModelUntouchedUntilRelease) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  EditorContext ctx;

  ASSERT_TRUE(actor.onDragStart(ctx, Vec2{22, 22}));
  actor.onDrag(ctx, Vec2{}, Vec2{10, 0});  // 10px -> 2 grid units wider
  actor.layout(ctx);

  EXPECT_EQ(actor.bounds().w, 35.0) << "the preview grows on screen";
  EXPECT_EQ(actor.location()->width, 5) << "the model only grows on commit";
}

TEST(NodeResize, ReleaseRaisesOneResizeForTheWholeGesture) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = [&](std::unique_ptr<Transaction> edit) {
    edits.push_back(std::move(edit));
    return true;
  };

  ASSERT_TRUE(actor.onDragStart(ctx, Vec2{22, 22}));
  actor.onDrag(ctx, Vec2{}, Vec2{10, 0});
  actor.onDrag(ctx, Vec2{}, Vec2{5, 5});
  actor.onDragEnd(ctx, Vec2{});

  ASSERT_EQ(edits.size(), 1u);

  GraphScene scene;
  scene.build(ctx, makeTree());
  ASSERT_TRUE(edits[0]->execute(scene));
  const fluir::FlowGraphLocation* resized = scene.find(kFunctionId, 1)->location();
  ASSERT_NE(resized, nullptr);
  EXPECT_EQ(resized->width, 8);
  EXPECT_EQ(resized->height, 5) << "a horizontal drag leaves the height alone";
}

TEST(NodeResize, AReleaseThatResizedNothingRaisesNoEdit) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = [&](std::unique_ptr<Transaction> edit) {
    edits.push_back(std::move(edit));
    return true;
  };

  ASSERT_TRUE(actor.onDragStart(ctx, Vec2{22, 22}));
  actor.onDrag(ctx, Vec2{}, Vec2{3, 3});  // sub-grid: nothing resized
  actor.onDragEnd(ctx, Vec2{});

  EXPECT_TRUE(edits.empty());
}

TEST(NodeResize, CancelDropsThePreview) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = [&](std::unique_ptr<Transaction> edit) {
    edits.push_back(std::move(edit));
    return true;
  };

  ASSERT_TRUE(actor.onDragStart(ctx, Vec2{22, 22}));
  actor.onDrag(ctx, Vec2{}, Vec2{10, 0});
  actor.onDragCancel();
  actor.layout(ctx);

  EXPECT_EQ(actor.bounds(), (Rect{0, 0, 25, 25}));
  EXPECT_TRUE(edits.empty());
}

TEST(NodeResize, PreviewClampsToTheMinimumWidth) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = [&](std::unique_ptr<Transaction> edit) {
    edits.push_back(std::move(edit));
    return true;
  };

  ASSERT_TRUE(actor.onDragStart(ctx, Vec2{22, 22}));
  actor.onDrag(ctx, Vec2{}, Vec2{-100, 0});  // -20 grid units, far past zero
  actor.layout(ctx);

  EXPECT_EQ(actor.bounds().w, 4 * ctx.layout.unitPx);

  actor.onDragEnd(ctx, Vec2{});
  ASSERT_EQ(edits.size(), 1u);

  GraphScene scene;
  scene.build(ctx, makeTree());
  ASSERT_TRUE(edits[0]->execute(scene));
  EXPECT_EQ(scene.find(kFunctionId, 1)->location()->width, 4);
}

TEST(NodeResize, DrawRendersTheResizeBarFromResizeRect) {
  const FlowGraphLocation loc{.x = 2, .y = 2, .z = 0, .width = 5, .height = 5};
  BinaryActor actor(kFunctionId, makeBinary(loc), Rect{0, 0, 25, 25});
  const EditorContext ctx;

  // nodeRect = localRect(loc, unitPx) = {10, 10, 25, 25}; resizeRect is grid
  // units {loc.width - WIDTH, 0, WIDTH, loc.height} = {4, 0, 1, 5}
  // -> world {10 + 4*5, 10 + 0, 1*5, 5*5} = {30, 10, 5, 25}.
  const double u = ctx.layout.unitPx;
  const Rect nodeRect{loc.x * u, loc.y * u, loc.width * u, loc.height * u};
  const Rect rr = fluir::editor::HorizResizeHandle{}.rect(loc);
  const Rect bar{nodeRect.x + rr.x * u, nodeRect.y + rr.y * u, rr.w * u, rr.h * u};

  EXPECT_EQ(bar, (Rect{30, 10, 5, 25}));
  EXPECT_TRUE(hasFill(recordDraw(actor, ctx), bar));
}
