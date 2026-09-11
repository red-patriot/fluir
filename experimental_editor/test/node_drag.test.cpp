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
#include "editor/components/drag_handle.hpp"
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

// handle span = min(3 * unitPx(5), w, h) = min(15, 25, 25) = 15, anchored at
// the node's top-left -> world rect {0, 0, 15, 15}.
TEST(NodeDrag, OnDragStartClaimsOnlyOnHandle) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  const EditorContext ctx;

  EXPECT_TRUE(actor.onDragStart(ctx, Vec2{5, 5}));       // inside the handle
  EXPECT_FALSE(actor.onDragStart(ctx, Vec2{20, 20}));    // inside the node, off the handle
  EXPECT_FALSE(actor.onDragStart(ctx, Vec2{100, 100}));  // off the node entirely
}

TEST(NodeDrag, OnDragGridSnapsBounds) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  const EditorContext ctx;

  ASSERT_TRUE(actor.onDragStart(ctx, Vec2{5, 5}));
  actor.onDrag(ctx, Vec2{}, Vec2{12, 3});  // 12/5 -> 2 units (10px); 3/5 -> 0 units
  actor.layout(ctx);                       // bounds follow the location, no draw needed

  EXPECT_EQ(actor.bounds(), (Rect{10, 0, 25, 25}));
  EXPECT_TRUE(hasFill(recordDraw(actor, ctx), Rect{10, 0, 25, 25}));
}

TEST(NodeDrag, OnDragAccumulatesSubGridRemainder) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  const EditorContext ctx;

  ASSERT_TRUE(actor.onDragStart(ctx, Vec2{5, 5}));

  actor.onDrag(ctx, Vec2{}, Vec2{3, 0});  // 3px < 1 unit -> no move yet
  actor.layout(ctx);
  EXPECT_EQ(actor.bounds().x, 0.0);

  actor.onDrag(ctx, Vec2{}, Vec2{3, 0});  // 6px total -> 1 whole unit (5px)
  actor.layout(ctx);
  EXPECT_EQ(actor.bounds().x, 5.0);
}

TEST(NodeDrag, DrawRendersHandleBlockFromDragRect) {
  const FlowGraphLocation loc{.x = 2, .y = 2, .z = 0, .width = 5, .height = 5};
  BinaryActor actor(kFunctionId, makeBinary(loc), Rect{0, 0, 25, 25});
  const EditorContext ctx;

  // draw() computes nodeRect = localRect(loc, unitPx) = {10, 10, 25, 25}, then
  // fills dragRect(loc) scaled into it: dragRect is grid units
  //   {loc.width - (WIDTH + 1), 1, WIDTH, HEIGHT} = {1, 1, 3, 3}
  // -> world {10 + 1*5, 10 + 1*5, 3*5, 3*5} = {15, 15, 15, 15}.
  const double u = ctx.layout.unitPx;
  const Rect nodeRect{loc.x * u, loc.y * u, loc.width * u, loc.height * u};
  const Rect dr = fluir::editor::dragRect(loc);
  const Rect handle{nodeRect.x + dr.x * u, nodeRect.y + dr.y * u, dr.w * u, dr.h * u};

  EXPECT_EQ(handle, (Rect{15, 15, 15, 15}));
  EXPECT_TRUE(hasFill(recordDraw(actor, ctx), handle));
}

TEST(NodeDrag, HandleBlockFitsWithinNode) {
  // Smallest node a real graph uses is 5x5 units (fixtures declare w="5" h="5").
  const FlowGraphLocation loc{.x = 0, .y = 0, .z = 0, .width = 5, .height = 5};
  BinaryActor actor(kFunctionId, makeBinary(loc), Rect{0, 0, 25, 25});
  const EditorContext ctx;

  const double u = ctx.layout.unitPx;
  const Rect nodeRect{loc.x * u, loc.y * u, loc.width * u, loc.height * u};
  const Rect dr = fluir::editor::dragRect(loc);
  const Rect handle{nodeRect.x + dr.x * u, nodeRect.y + dr.y * u, dr.w * u, dr.h * u};

  // The handle is a fixed 3x3-unit (15x15 px) grip inset one unit from the
  // node's top and right edges, so it never overflows the node rect.
  EXPECT_EQ(handle, (Rect{5, 5, 15, 15}));
  EXPECT_GE(handle.x, nodeRect.x);
  EXPECT_GE(handle.y, nodeRect.y);
  EXPECT_LE(handle.x + handle.w, nodeRect.x + nodeRect.w);
  EXPECT_LE(handle.y + handle.h, nodeRect.y + nodeRect.h);

  const auto calls = recordDraw(actor, ctx);
  EXPECT_TRUE(hasFill(calls, handle));
  // The pre-refactor handle was a square anchored at the node's top-left corner.
  EXPECT_FALSE(hasFill(calls, Rect{0, 0, 15, 15}));
}

TEST(NodeDrag, ADragLeavesTheModelUntouchedUntilRelease) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  EditorContext ctx;

  ASSERT_TRUE(actor.onDragStart(ctx, Vec2{5, 5}));
  actor.onDrag(ctx, Vec2{}, Vec2{10, 0});  // 10px -> 2 grid units
  actor.layout(ctx);

  EXPECT_EQ(actor.bounds().x, 10.0) << "the preview moves on screen";
  EXPECT_EQ(actor.location()->x, 0) << "the model only moves on commit";
}

TEST(NodeDrag, ReleaseRaisesOneMoveForTheWholeGesture) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = [&](std::unique_ptr<Transaction> edit) {
    edits.push_back(std::move(edit));
    return true;
  };

  ASSERT_TRUE(actor.onDragStart(ctx, Vec2{5, 5}));
  actor.onDrag(ctx, Vec2{}, Vec2{5, 0});
  actor.onDrag(ctx, Vec2{}, Vec2{5, 5});
  actor.onDragEnd(ctx, Vec2{});

  ASSERT_EQ(edits.size(), 1u);

  GraphScene scene;
  scene.build(ctx, makeTree());
  ASSERT_TRUE(edits[0]->execute(scene));
  const fluir::FlowGraphLocation* moved = scene.find(kFunctionId, 1)->location();
  ASSERT_NE(moved, nullptr);
  EXPECT_EQ(moved->x, 2);
  EXPECT_EQ(moved->y, 1);
}

TEST(NodeDrag, AReleaseThatMovedNothingRaisesNoEdit) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = [&](std::unique_ptr<Transaction> edit) {
    edits.push_back(std::move(edit));
    return true;
  };

  ASSERT_TRUE(actor.onDragStart(ctx, Vec2{5, 5}));
  actor.onDrag(ctx, Vec2{}, Vec2{3, 3});  // sub-grid: nothing moved
  actor.onDragEnd(ctx, Vec2{});

  EXPECT_TRUE(edits.empty());
}

TEST(NodeDrag, CancelDropsThePreview) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = [&](std::unique_ptr<Transaction> edit) {
    edits.push_back(std::move(edit));
    return true;
  };

  ASSERT_TRUE(actor.onDragStart(ctx, Vec2{5, 5}));
  actor.onDrag(ctx, Vec2{}, Vec2{10, 10});
  actor.onDragCancel();
  actor.layout(ctx);

  EXPECT_EQ(actor.bounds(), (Rect{0, 0, 25, 25}));
  EXPECT_TRUE(edits.empty());
}
