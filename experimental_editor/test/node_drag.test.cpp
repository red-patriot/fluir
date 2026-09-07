#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
#include "editor/actors/node_actors.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"
#include "recording_renderer.hpp"

namespace {

  using fluir::FlowGraphLocation;
  using fluir::ID;
  using fluir::Operator;
  using fluir::editor::BinaryActor;
  using fluir::editor::EditorContext;
  using fluir::editor::Rect;
  using fluir::editor::Subview;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::hasFill;
  using testutil::RecordingRenderer;

  constexpr ID kFunctionId = 100;

  // width/height = 5 units, unitPx = 5 (EditorContext default) -> a 25x25
  // local-space node rect, matching the initial bounds the tests pass in.
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
  std::vector<testutil::DrawCall> recordDraw(const BinaryActor& actor, const EditorContext& ctx) {
    RecordingRenderer renderer;
    const Viewport viewport;
    {
      const Subview body{viewport, Rect{0, 0, 1000, 1000}, renderer};
      actor.draw(body, ctx);
    }
    return renderer.calls;
  }

}  // namespace

// handle span = min(3 * unitPx(5), w, h) = min(15, 25, 25) = 15, anchored at
// the node's top-left -> world rect {0, 0, 15, 15}.
TEST(NodeDrag, OnDragStartClaimsOnlyOnHandle) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  const EditorContext ctx;

  EXPECT_TRUE(actor.onDragStart(ctx, Vec2{5, 5}));       // inside the handle
  EXPECT_FALSE(actor.onDragStart(ctx, Vec2{20, 20}));    // inside the node, off the handle
  EXPECT_FALSE(actor.onDragStart(ctx, Vec2{100, 100}));  // off the node entirely
}

TEST(NodeDrag, OnDragGridSnapsLocationAndBounds) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  const EditorContext ctx;

  ASSERT_TRUE(actor.onDragStart(ctx, Vec2{5, 5}));
  actor.onDrag(ctx, Vec2{}, Vec2{12, 3});  // 12/5 -> 2 units (10px); 3/5 -> 0 units

  EXPECT_EQ(actor.bounds(), (Rect{10, 0, 25, 25}));
  EXPECT_TRUE(hasFill(recordDraw(actor, ctx), Rect{10, 0, 25, 25}));
}

TEST(NodeDrag, OnDragAccumulatesSubGridRemainder) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  const EditorContext ctx;

  ASSERT_TRUE(actor.onDragStart(ctx, Vec2{5, 5}));

  actor.onDrag(ctx, Vec2{}, Vec2{3, 0});  // 3px < 1 unit -> no move yet
  EXPECT_EQ(actor.bounds().x, 0.0);

  actor.onDrag(ctx, Vec2{}, Vec2{3, 0});  // 6px total -> 1 whole unit (5px)
  EXPECT_EQ(actor.bounds().x, 5.0);
}

TEST(NodeDrag, DrawRendersHandleBlockAtNodeTopLeft) {
  BinaryActor actor(
    kFunctionId, makeBinary(FlowGraphLocation{.x = 2, .y = 2, .z = 0, .width = 5, .height = 5}), Rect{0, 0, 25, 25});
  const EditorContext ctx;

  // nodeRect = {10, 10, 25, 25}; handle span = min(15, 25, 25) = 15.
  EXPECT_TRUE(hasFill(recordDraw(actor, ctx), Rect{10, 10, 15, 15}));
}

TEST(NodeDrag, HandleClampsToSmallNode) {
  BinaryActor actor(
    kFunctionId, makeBinary(FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 2, .height = 5}), Rect{0, 0, 10, 25});
  const EditorContext ctx;

  // nodeRect = {0, 0, 10, 25}; handle span = min(15, 10, 25) = 10, not 15.
  const auto calls = recordDraw(actor, ctx);
  EXPECT_TRUE(hasFill(calls, Rect{0, 0, 10, 10}));
  EXPECT_FALSE(hasFill(calls, Rect{0, 0, 15, 15}));
}
