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
#include "editor/core/layer.hpp"
#include "editor/core/loader.hpp"
#include "editor/core/viewport.hpp"
#include "fixture_loader.hpp"
#include "recording_renderer.hpp"

// Dragging a function frame's DragHandle must move the whole frame as one unit:
// The frame, rails, body nodes and conduits all hang off the FunctionDeclActor's
// live location_ (which the DragHandle mutates), not the stale parse-tree
// decl.location. Contract style: assert observable screen
// geometry (rects, text positions, clip rects), never draw-call counts.

namespace {

  using testutil::Loaded;
  using testutil::loadFixture;

  namespace fs = std::filesystem;

  using fluir::editor::Actor;
  using fluir::editor::FunctionDeclActor;
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
    scene.layout(kCtx);  // layout, not draw, is what reconciles bounds with the new location
  }

  // Draws `scene` through one Layer with an identity viewport, as ModulePage does.
  void drawScene(const GraphScene& scene, RecordingRenderer& r) {
    fluir::editor::Layer layer;
    layer.setRoot(scene.root());
    layer.draw(r, kCtx, Rect{0, 0, r.outputSize().x, r.outputSize().y});
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
// frame: the body container carries them, so they move with its bounds.
TEST(FunctionDeclDrag, BodyNodesFollowFrameDrag) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  RecordingRenderer before;
  drawScene(scene, before);
  // binary id=1 pre-drag: bodyOrigin {50,75} + localRect {75,10,25,25}.
  ASSERT_TRUE(hasRect(before.calls, Rect{125, 85, 25, 25}));

  dragFrame(scene, 10, 4);  // +50 world px in x, +20 world px in y

  RecordingRenderer after;
  drawScene(scene, after);

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
  drawScene(scene, before);
  // name label pre-drag: header origin {50,50} + textPad {4,4}.
  ASSERT_TRUE(hasTextAt(before.calls, "foo", Vec2{54, 54}));

  dragFrame(scene, 10, 4);

  RecordingRenderer after;
  drawScene(scene, after);

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
  drawScene(scene, r);

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
  ASSERT_EQ(node->worldBounds(), (Rect{125, 85, 25, 25}));

  dragFrame(scene, 10, 4);  // +50 world px in x, +20 in y
  RecordingRenderer after;
  drawScene(scene, after);

  // The pick rect must shift by the same (50,20) world delta as the drawn rect.
  EXPECT_EQ(node->worldBounds(), (Rect{175, 105, 25, 25}));
}

TEST(FunctionDeclDrag, BodyNodeHitBoundsMatchDrawnRectAfterFrameDrag) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  dragFrame(scene, 10, 4);
  RecordingRenderer after;
  drawScene(scene, after);

  Actor* node = scene.find(1, 1);
  ASSERT_NE(node, nullptr);
  EXPECT_TRUE(hasRect(after.calls, node->worldBounds()));
}

TEST(FunctionDeclDrag, BodyNodeOwnDragBoundsSurviveRerender) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  Actor* node = scene.find(1, 1);
  ASSERT_NE(node, nullptr);
  // binary id=1 world rect {125,85,25,25}; its grip is handleBox({125,85,..},{1,1,3,3}) = {130,90,15,15}.
  // Events arrive in the node's parent (body) space, so the world point converts first.
  ASSERT_TRUE(node->onDragStart(kCtx, node->toParentLocal(Vec2{137, 97})));
  node->onDrag(kCtx, Vec2{}, Vec2{2 * kCtx.layout.unitPx, 0});  // +2 units in x
  scene.layout(kCtx);
  const Rect dragged = node->worldBounds();
  ASSERT_EQ(dragged, (Rect{135, 85, 25, 25}));

  RecordingRenderer after;
  drawScene(scene, after);

  EXPECT_EQ(node->worldBounds(), dragged);
}

TEST(FunctionDeclDrag, AtRestRenderDoesNotMoveBodyNodeBounds) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  Actor* node = scene.find(1, 1);
  ASSERT_NE(node, nullptr);
  const Rect built = node->worldBounds();

  RecordingRenderer r1;
  drawScene(scene, r1);
  RecordingRenderer r2;
  drawScene(scene, r2);

  EXPECT_EQ(node->worldBounds(), built);
}
