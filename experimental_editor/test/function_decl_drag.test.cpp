#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/utility/context.hpp"
#include "editor/actors/function_decl_actor.hpp"
#include "editor/actors/scene.hpp"
#include "editor/core/collecting_sink.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/loader.hpp"
#include "editor/core/render_graph.hpp"
#include "editor/core/viewport.hpp"
#include "recording_renderer.hpp"

// Dragging a function frame's DragHandle must move the whole frame as one unit:
// GraphRenderer has to place the frame, rails, body nodes and conduits from the
// FunctionDeclActor's live location_ (which the DragHandle mutates), not the
// stale parse-tree decl.location. Contract style: assert observable screen
// geometry (rects, text positions, clip rects), never draw-call counts.

namespace {

  namespace fs = std::filesystem;

  using fluir::editor::Actor;
  using fluir::editor::FunctionDeclActor;
  using fluir::editor::GraphRenderer;
  using fluir::editor::GraphScene;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::DrawCall;
  using testutil::expectRectNear;
  using testutil::hasRect;
  using testutil::hasTextAt;
  using testutil::opsOf;
  using testutil::RecordingRenderer;

  const fluir::editor::EditorContext kCtx;

  // Same fixture-loading shape as render_graph.test.cpp / scene.test.cpp.
  struct Loaded {
    fluir::editor::CollectingSink sink;
    fluir::editor::LoadResult result;
  };

  Loaded loadFixture(const std::string& relPath) {
    Loaded l;
    fluir::Context ctx{
      .diagnosticSink = l.sink,
      .symbolTable = {},
      .currentFile = {},
      .outputFilename = {},
      .version = {},
      .ignoreVersionChecks = true,
    };
    l.result = fluir::editor::loadFile(ctx, fs::path(TEST_FOLDER) / relPath);
    return l;
  }

  // Drags function id=1's frame handle by (dxUnits, dyUnits) logical units.
  // simple_binary_expr.fl / single_empty_function.fl: foo at x=10 y=10 w=100
  // h=100; unitPx 5 -> frame bounds {50,50,500,500} and drag-handle world box
  // {530,55,15,15}. DragHandle::onDrag takes a *world-pixel* delta and snaps to
  // whole units, so a delta of dxUnits*unitPx moves the frame exactly dxUnits.
  void dragFrame(GraphScene& scene, int dxUnits, int dyUnits) {
    auto* frame = dynamic_cast<FunctionDeclActor*>(scene.find(1));
    ASSERT_NE(frame, nullptr);
    ASSERT_TRUE(frame->onDragStart(kCtx, Vec2{537.5, 62.5}));
    frame->onDrag(
      kCtx,
      Vec2{},
      Vec2{static_cast<double>(dxUnits) * kCtx.layout.unitPx, static_cast<double>(dyUnits) * kCtx.layout.unitPx});
  }

  long firstIndex(const std::vector<DrawCall>& calls, bool (*pred)(const DrawCall&)) {
    for (std::size_t i = 0; i < calls.size(); ++i) {
      if (pred(calls[i])) {
        return static_cast<long>(i);
      }
    }
    return -1;
  }

}  // namespace

// Bug 1: the body node (and, with it, rails / conduits) must track the dragged
// frame. Today GraphRenderer re-derives the body origin from decl.location, so
// the child stays put while the header moves.
TEST(FunctionDeclDrag, BodyNodesFollowFrameDrag) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  RecordingRenderer before;
  GraphRenderer{kCtx, Viewport{}, before, scene}(*l.result.tree);
  // binary id=1 pre-drag: bodyOrigin {50,75} + localRect {75,10,25,25}.
  ASSERT_TRUE(hasRect(before.calls, Rect{125, 85, 25, 25}));

  dragFrame(scene, 10, 4);  // +50 world px in x, +20 world px in y

  RecordingRenderer after;
  GraphRenderer{kCtx, Viewport{}, after, scene}(*l.result.tree);

  // The child node's drawn rect must shift by the same (50,20) world delta.
  EXPECT_TRUE(hasRect(after.calls, Rect{175, 105, 25, 25}));
  EXPECT_FALSE(hasRect(after.calls, Rect{125, 85, 25, 25}));
}

// Bug in the name-text placement: the label is drawn at the world origin plus
// textPad, so it neither sits on the header nor tracks the drag.
TEST(FunctionDeclDrag, FrameNameTracksFrameDrag) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  RecordingRenderer before;
  GraphRenderer{kCtx, Viewport{}, before, scene}(*l.result.tree);
  // name label pre-drag: header origin {50,50} + textPad {4,4}.
  ASSERT_TRUE(hasTextAt(before.calls, "foo", Vec2{54, 54}));

  dragFrame(scene, 10, 4);

  RecordingRenderer after;
  GraphRenderer{kCtx, Viewport{}, after, scene}(*l.result.tree);

  // name label must move with the frame: header origin {100,70} + textPad.
  EXPECT_TRUE(hasTextAt(after.calls, "foo", Vec2{104, 74}));
}

// Bug 2 regression: the frame chrome must draw under the full-output clip, not
// be scissored to the frame's own (pre-drag) rectangle. Passes with the
// on-disk Subview reorder; lock it in.
TEST(FunctionDeclDrag, FrameChromeIsNotScissoredToItsOwnRect) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  RecordingRenderer r;  // default output size 800x600
  GraphRenderer{kCtx, Viewport{}, r, scene}(*l.result.tree);

  const auto pushes = opsOf(r.calls, DrawCall::Op::PushClip);
  ASSERT_FALSE(pushes.empty());
  // First clip pushed is the whole canvas -- the chrome draws inside only this.
  expectRectNear(pushes.front().rect, Rect{0, 0, 800, 600});

  const long headerFill = firstIndex(
    r.calls, [](const DrawCall& c) { return c.op == DrawCall::Op::Fill && c.rect == Rect{50, 50, 500, 25}; });
  const long frameClip = firstIndex(
    r.calls, [](const DrawCall& c) { return c.op == DrawCall::Op::PushClip && c.rect == Rect{50, 50, 500, 500}; });
  ASSERT_GE(headerFill, 0);
  // The frame-sized clip, if pushed at all, comes only after the header is drawn.
  EXPECT_TRUE(frameClip < 0 || headerFill < frameClip);
}

TEST(FunctionDeclDrag, BodyNodeHitBoundsFollowFrameDrag) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  Actor* node = scene.find(1, 1);  // binary id=1 in function id=1
  ASSERT_NE(node, nullptr);
  // Matches the drawn rect: bodyOrigin {50,75} + localRect {75,10,25,25}.
  ASSERT_EQ(node->bounds(), (Rect{125, 85, 25, 25}));

  dragFrame(scene, 10, 4);  // +50 world px in x, +20 in y
  RecordingRenderer after;
  GraphRenderer{kCtx, Viewport{}, after, scene}(*l.result.tree);

  // The pick rect must shift by the same (50,20) world delta as the drawn rect.
  EXPECT_EQ(node->bounds(), (Rect{175, 105, 25, 25}));
}

TEST(FunctionDeclDrag, BodyNodeHitBoundsMatchDrawnRectAfterFrameDrag) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  dragFrame(scene, 10, 4);
  RecordingRenderer after;
  GraphRenderer{kCtx, Viewport{}, after, scene}(*l.result.tree);

  Actor* node = scene.find(1, 1);
  ASSERT_NE(node, nullptr);
  EXPECT_TRUE(hasRect(after.calls, node->bounds()));
}

TEST(FunctionDeclDrag, BodyNodeOwnDragBoundsSurviveRerender) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  Actor* node = scene.find(1, 1);
  ASSERT_NE(node, nullptr);
  // binary id=1 world rect {125,85,25,25}; its grip is handleBox({125,85,..},{1,1,3,3}) = {130,90,15,15}.
  ASSERT_TRUE(node->onDragStart(kCtx, Vec2{137, 97}));
  node->onDrag(kCtx, Vec2{}, Vec2{2 * kCtx.layout.unitPx, 0});  // +2 units in x
  const Rect dragged = node->bounds();
  ASSERT_EQ(dragged, (Rect{135, 85, 25, 25}));

  RecordingRenderer after;
  GraphRenderer{kCtx, Viewport{}, after, scene}(*l.result.tree);

  EXPECT_EQ(node->bounds(), dragged);
}

TEST(FunctionDeclDrag, AtRestRenderDoesNotMoveBodyNodeBounds) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  Actor* node = scene.find(1, 1);
  ASSERT_NE(node, nullptr);
  const Rect built = node->bounds();

  RecordingRenderer r1;
  GraphRenderer{kCtx, Viewport{}, r1, scene}(*l.result.tree);
  RecordingRenderer r2;
  GraphRenderer{kCtx, Viewport{}, r2, scene}(*l.result.tree);

  EXPECT_EQ(node->bounds(), built);
}
