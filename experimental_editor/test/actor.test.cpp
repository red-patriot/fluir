#include "editor/actors/actor.hpp"

#include <cstddef>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
#include "editor/actors/function_decl_actor.hpp"
#include "editor/actors/node_actors.hpp"
#include "editor/components/container_actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"
#include "recording_renderer.hpp"

namespace {

  using fluir::FlowGraphLocation;
  using fluir::ID;
  using fluir::Operator;
  using fluir::editor::Actor;
  using fluir::editor::BinaryActor;
  using fluir::editor::CallActor;
  using fluir::editor::ConstantActor;
  using fluir::editor::EditorContext;
  using fluir::editor::FunctionDeclActor;
  using fluir::editor::PortSet;
  using fluir::editor::Rect;
  using fluir::editor::Subview;
  using fluir::editor::UnaryActor;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::fillsOfSize;
  using testutil::hasFill;
  using testutil::hasRect;
  using testutil::hasTextAt;
  using testutil::RecordingRenderer;

  // Geometry fields are irrelevant to click dispatch; z is unused.
  const FlowGraphLocation kLoc{.x = 0, .y = 0, .z = 0, .width = 0, .height = 0};

  constexpr ID kFunctionId = 100;

  // Non-degenerate location for draw tests: width/height = 5 units, unitPx = 5
  // (EditorContext default) -> a 25x25 local-space node rect.
  const FlowGraphLocation kDrawLoc{.x = 0, .y = 0, .z = 0, .width = 5, .height = 5};

  fluir::pt::Binary makeBinary() {
    fluir::pt::Binary binary;
    binary.id = 1;
    binary.location = kLoc;
    binary.lhs = 2;
    binary.rhs = 3;
    binary.op = Operator::PLUS;
    return binary;
  }

  fluir::pt::Unary makeUnary() {
    fluir::pt::Unary unary;
    unary.id = 4;
    unary.location = kLoc;
    unary.lhs = 5;
    unary.op = Operator::MINUS;
    return unary;
  }

  fluir::pt::Constant makeConstant() {
    fluir::pt::Constant constant;
    constant.id = 6;
    constant.location = kLoc;
    constant.value = fluir::literals_types::I32{42};
    return constant;
  }

  fluir::pt::Call makeCall() {
    fluir::pt::Call call;
    call.id = 7;
    call.location = kLoc;
    call.target = "add";
    return call;
  }

  // width=5, height=10 (distinct from width) so header/frame draw rects differ;
  // unitPx=5 (EditorContext default) -> w=25, h=50, headerH=25.
  fluir::pt::FunctionDecl makeFunctionDecl() {
    fluir::pt::FunctionDecl decl;
    decl.id = 8;
    decl.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 5, .height = 10};
    decl.name = "foo";
    return decl;
  }

}  // namespace

TEST(Actor, BinaryActorOnClickSummarizesOperator) {
  BinaryActor actor(kFunctionId, makeBinary(), Rect{0, 0, 10, 10});
  actor.onClick(Vec2{1, 1});

  EXPECT_NE(actor.lastClickSummary().find("+"), std::string::npos);
}

TEST(Actor, UnaryActorOnClickSummarizesOperator) {
  UnaryActor actor(kFunctionId, makeUnary(), Rect{0, 0, 10, 10});
  actor.onClick(Vec2{1, 1});

  EXPECT_NE(actor.lastClickSummary().find("-"), std::string::npos);
}

TEST(Actor, ConstantActorOnClickSummarizesValue) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 10, 10});
  actor.onClick(Vec2{1, 1});

  EXPECT_NE(actor.lastClickSummary().find("42"), std::string::npos);
}

TEST(Actor, CallActorOnClickSummarizesTarget) {
  CallActor actor(kFunctionId, makeCall(), Rect{0, 0, 10, 10});
  actor.onClick(Vec2{1, 1});

  EXPECT_NE(actor.lastClickSummary().find("add"), std::string::npos);
}

// Virtual dispatch smoke test: each concrete actor is reachable and produces
// its own type-specific summary when clicked through a base `Actor*`, not
// just when clicked directly on the concrete type.
TEST(Actor, VirtualDispatchReachesEachOverride) {
  std::unique_ptr<Actor> binary = std::make_unique<BinaryActor>(kFunctionId, makeBinary(), Rect{0, 0, 10, 10});
  std::unique_ptr<Actor> unary = std::make_unique<UnaryActor>(kFunctionId, makeUnary(), Rect{0, 0, 10, 10});
  std::unique_ptr<Actor> constant = std::make_unique<ConstantActor>(kFunctionId, makeConstant(), Rect{0, 0, 10, 10});
  std::unique_ptr<Actor> call = std::make_unique<CallActor>(kFunctionId, makeCall(), Rect{0, 0, 10, 10});

  binary->onClick(Vec2{1, 1});
  unary->onClick(Vec2{1, 1});
  constant->onClick(Vec2{1, 1});
  call->onClick(Vec2{1, 1});

  EXPECT_NE(static_cast<BinaryActor&>(*binary).lastClickSummary().find("+"), std::string::npos);
  EXPECT_NE(static_cast<UnaryActor&>(*unary).lastClickSummary().find("-"), std::string::npos);
  EXPECT_NE(static_cast<ConstantActor&>(*constant).lastClickSummary().find("42"), std::string::npos);
  EXPECT_NE(static_cast<CallActor&>(*call).lastClickSummary().find("add"), std::string::npos);
}

TEST(Actor, IdAndBoundsReturnConstructionValues) {
  const Rect bounds{10, 20, 30, 40};
  BinaryActor actor(kFunctionId, makeBinary(), bounds);

  EXPECT_EQ(actor.id(), (fluir::FullID{kFunctionId, 1}));
  EXPECT_EQ(actor.bounds(), bounds);
}

// Draw tests use identity Viewport + a root Subview at world origin, so
// `body.toScreen(local) == local`; kDrawLoc * unitPx(5) -> node rect {0,0,25,25}.

TEST(Actor, BinaryActorDrawsBodyLabelAndPorts) {
  fluir::pt::Binary node = makeBinary();
  node.location = kDrawLoc;
  BinaryActor actor(kFunctionId, node, Rect{0, 0, 10, 10});

  const EditorContext ctx;
  actor.layout(ctx);
  RecordingRenderer renderer;
  const Viewport viewport;
  {
    const Subview body{viewport, Rect{0, 0, 1000, 1000}, renderer};
    actor.draw(body, ctx);
  }
  const PortSet ports = actor.ports(ctx);

  EXPECT_TRUE(hasFill(renderer.calls, Rect{0, 0, 25, 25}));
  EXPECT_TRUE(hasRect(renderer.calls, Rect{0, 0, 25, 25}));
  EXPECT_TRUE(hasTextAt(renderer.calls, "+", Vec2{4, 4}));

  const auto dots = fillsOfSize(renderer.calls, 6, 6);
  EXPECT_EQ(dots.size(), 3u);  // 2 inputs + 1 output

  ASSERT_EQ(ports.inputs.size(), 2u);
  EXPECT_EQ(ports.inputs[0], (Vec2{0, 0}));
  EXPECT_EQ(ports.inputs[1], (Vec2{0, 25}));
  ASSERT_EQ(ports.outputs.size(), 1u);
  EXPECT_EQ(ports.outputs[0], (Vec2{25, 12.5}));
}

TEST(Actor, UnaryActorDrawsBodyLabelAndPorts) {
  fluir::pt::Unary node = makeUnary();
  node.location = kDrawLoc;
  UnaryActor actor(kFunctionId, node, Rect{0, 0, 10, 10});

  const EditorContext ctx;
  actor.layout(ctx);
  RecordingRenderer renderer;
  const Viewport viewport;
  {
    const Subview body{viewport, Rect{0, 0, 1000, 1000}, renderer};
    actor.draw(body, ctx);
  }
  const PortSet ports = actor.ports(ctx);

  EXPECT_TRUE(hasFill(renderer.calls, Rect{0, 0, 25, 25}));
  EXPECT_TRUE(hasRect(renderer.calls, Rect{0, 0, 25, 25}));
  EXPECT_TRUE(hasTextAt(renderer.calls, "-", Vec2{4, 4}));

  const auto dots = fillsOfSize(renderer.calls, 6, 6);
  EXPECT_EQ(dots.size(), 2u);  // 1 input + 1 output

  ASSERT_EQ(ports.inputs.size(), 1u);
  EXPECT_EQ(ports.inputs[0], (Vec2{0, 12.5}));
  ASSERT_EQ(ports.outputs.size(), 1u);
  EXPECT_EQ(ports.outputs[0], (Vec2{25, 12.5}));
}

TEST(Actor, ConstantActorDrawsBodyLabelAndOutputPort) {
  fluir::pt::Constant node = makeConstant();
  node.location = kDrawLoc;
  ConstantActor actor(kFunctionId, node, Rect{0, 0, 10, 10});

  const EditorContext ctx;
  actor.layout(ctx);
  RecordingRenderer renderer;
  const Viewport viewport;
  {
    const Subview body{viewport, Rect{0, 0, 1000, 1000}, renderer};
    actor.draw(body, ctx);
  }
  const PortSet ports = actor.ports(ctx);

  EXPECT_TRUE(hasFill(renderer.calls, Rect{0, 0, 25, 25}));
  EXPECT_TRUE(hasRect(renderer.calls, Rect{0, 0, 25, 25}));
  EXPECT_TRUE(hasTextAt(renderer.calls, "42", Vec2{4, 4}));

  const auto dots = fillsOfSize(renderer.calls, 6, 6);
  EXPECT_EQ(dots.size(), 1u);  // output only

  EXPECT_TRUE(ports.inputs.empty());
  ASSERT_EQ(ports.outputs.size(), 1u);
  EXPECT_EQ(ports.outputs[0], (Vec2{25, 12.5}));
}

TEST(Actor, CallActorDrawsBodyLabelArgRowsAndReturnPort) {
  fluir::pt::Call node = makeCall();
  node.location = kDrawLoc;
  fluir::pt::Call::Argument arg;
  arg.index = 0;
  arg.name = "x";
  node.arguments.push_back(arg);
  node._return = fluir::pt::Call::Return{};
  CallActor actor(kFunctionId, node, Rect{0, 0, 10, 10});

  const EditorContext ctx;
  actor.layout(ctx);
  RecordingRenderer renderer;
  const Viewport viewport;
  {
    const Subview body{viewport, Rect{0, 0, 1000, 1000}, renderer};
    actor.draw(body, ctx);
  }
  const PortSet ports = actor.ports(ctx);

  EXPECT_TRUE(hasFill(renderer.calls, Rect{0, 0, 25, 25}));
  EXPECT_TRUE(hasRect(renderer.calls, Rect{0, 0, 25, 25}));
  EXPECT_TRUE(hasTextAt(renderer.calls, "add", Vec2{4, 4}));
  // Arg row 0: rowTop = 0 + 1*railStep(25) = 25; label at (0+4, 25+4).
  EXPECT_TRUE(hasTextAt(renderer.calls, "x", Vec2{4, 29}));

  const auto dots = fillsOfSize(renderer.calls, 6, 6);
  EXPECT_EQ(dots.size(), 2u);  // 1 arg input + 1 return output

  ASSERT_EQ(ports.inputs.size(), 1u);
  EXPECT_EQ(ports.inputs[0], (Vec2{0, 25 + 12.5}));
  ASSERT_EQ(ports.outputs.size(), 1u);
  EXPECT_EQ(ports.outputs[0], (Vec2{25, 12.5}));
}

TEST(Actor, FunctionDeclActorOnClickSummarizesName) {
  FunctionDeclActor actor(makeFunctionDecl(), Rect{0, 0, 10, 10});
  actor.onClick(Vec2{1, 1});

  EXPECT_NE(actor.lastClickSummary().find("foo"), std::string::npos);
}

TEST(Actor, FunctionDeclActorFunctionIdAndBoundsReturnConstructionValues) {
  const Rect bounds{10, 20, 30, 40};
  FunctionDeclActor actor(makeFunctionDecl(), bounds);

  EXPECT_EQ(actor.functionId(), 8u);
  EXPECT_EQ(actor.bounds(), bounds);
}

namespace {

  // Minimal leaf that fills its own bounds in its parent's view.
  class StubActor : public fluir::editor::Actor {
   public:
    using Actor::Actor;

    void drawSelf(const Subview& view, const EditorContext&) const override {
      view.renderer().fillRect(view.toScreen(bounds()), {});
    }
  };

}  // namespace

TEST(ActorTree, AddSetsParentAndOwnsChild) {
  StubActor root{Rect{10, 20, 100, 100}};
  Actor& child = root.add(std::make_unique<StubActor>(Rect{5, 5, 10, 10}));

  ASSERT_EQ(root.children().size(), 1u);
  EXPECT_EQ(root.children().front().get(), &child);
  EXPECT_EQ(child.parent(), &root);
  EXPECT_EQ(root.parent(), nullptr);
}

TEST(ActorTree, WorldBoundsAccumulateThroughTwoLevels) {
  StubActor root{Rect{10, 20, 100, 100}};
  Actor& mid = root.add(std::make_unique<StubActor>(Rect{5, 5, 50, 50}));
  Actor& leaf = mid.add(std::make_unique<StubActor>(Rect{1, 2, 10, 10}));

  EXPECT_EQ(root.worldBounds(), (Rect{10, 20, 100, 100}));
  EXPECT_EQ(mid.worldBounds(), (Rect{15, 25, 50, 50}));
  EXPECT_EQ(leaf.worldBounds(), (Rect{16, 27, 10, 10}));
  EXPECT_EQ(leaf.toParentLocal(Vec2{16, 27}), (Vec2{1, 2}));
}

TEST(ActorTree, HitTestReturnsDeepestLastPaintedChild) {
  StubActor root{Rect{0, 0, 100, 100}};
  Actor& under = root.add(std::make_unique<StubActor>(Rect{10, 10, 40, 40}));
  Actor& over = root.add(std::make_unique<StubActor>(Rect{10, 10, 40, 40}));
  Actor& deep = over.add(std::make_unique<StubActor>(Rect{0, 0, 5, 5}));

  EXPECT_EQ(root.hitTest(Vec2{12, 12}), &deep);
  EXPECT_EQ(root.hitTest(Vec2{30, 30}), &over);
  EXPECT_NE(root.hitTest(Vec2{30, 30}), &under);
}

TEST(ActorTree, HitTestFallsBackToParentAndMissesOutside) {
  StubActor root{Rect{0, 0, 100, 100}};
  root.add(std::make_unique<StubActor>(Rect{10, 10, 10, 10}));

  EXPECT_EQ(root.hitTest(Vec2{80, 80}), &root);  // inside root, outside every child
  EXPECT_EQ(root.hitTest(Vec2{500, 500}), nullptr);
}

TEST(ActorTree, ContainerIsTransparentToHitTesting) {
  StubActor root{Rect{0, 0, 100, 100}};
  Actor& group = root.add(std::make_unique<fluir::editor::ContainerActor>(Rect{0, 0, 100, 100}));
  Actor& leaf = group.add(std::make_unique<StubActor>(Rect{10, 10, 10, 10}));

  EXPECT_EQ(root.hitTest(Vec2{12, 12}), &leaf);
  EXPECT_EQ(root.hitTest(Vec2{80, 80}), &root) << "an empty spot in a container must fall through to it";
}

TEST(ActorTree, DrawNestsChildrenInsideTheParentsClippedView) {
  StubActor root{Rect{10, 10, 100, 100}};
  root.add(std::make_unique<StubActor>(Rect{5, 5, 20, 20}));

  const EditorContext ctx;
  RecordingRenderer renderer;
  const Viewport viewport;
  {
    const Subview view{viewport, Rect{0, 0, 1000, 1000}, renderer};
    root.draw(view, ctx);
  }

  EXPECT_TRUE(hasFill(renderer.calls, Rect{10, 10, 100, 100}));  // root, in its parent's space
  EXPECT_TRUE(hasFill(renderer.calls, Rect{15, 15, 20, 20}));    // child, offset by root's origin
  EXPECT_EQ(testutil::clipsCovering(renderer.calls, Rect{10, 10, 100, 100}).size(), 1u);
}

TEST(Actor, FunctionDeclActorDrawsHeaderBorderAndName) {
  FunctionDeclActor actor(makeFunctionDecl(), Rect{0, 0, 10, 50});

  const EditorContext ctx;
  actor.layout(ctx);
  RecordingRenderer renderer;
  const Viewport viewport;
  {
    const Subview frame{viewport, Rect{0, 0, 1000, 1000}, renderer};
    actor.draw(frame, ctx);
  }

  EXPECT_TRUE(hasFill(renderer.calls, Rect{0, 0, 25, 25}));  // header band
  EXPECT_TRUE(hasRect(renderer.calls, Rect{0, 0, 25, 50}));  // frame border
  EXPECT_TRUE(hasTextAt(renderer.calls, "foo", Vec2{4, 4}));
}

// Detach/insert are the undo primitives: removal must hand ownership back, and
// re-insertion must restore draw order, not append on top.

TEST(ActorTree, DetachTransfersOwnershipAndClearsParent) {
  StubActor root{Rect{0, 0, 100, 100}};
  Actor& child = root.add(std::make_unique<StubActor>(Rect{5, 5, 10, 10}));

  std::unique_ptr<Actor> detached = root.detach(child);

  ASSERT_NE(detached, nullptr);
  EXPECT_EQ(detached.get(), &child);
  EXPECT_EQ(detached->parent(), nullptr);
  EXPECT_TRUE(root.children().empty());
}

TEST(ActorTree, DetachOfANonChildYieldsNothing) {
  StubActor root{Rect{0, 0, 100, 100}};
  StubActor stranger{Rect{0, 0, 10, 10}};
  root.add(std::make_unique<StubActor>(Rect{5, 5, 10, 10}));

  EXPECT_EQ(root.detach(stranger), nullptr);
  EXPECT_EQ(root.children().size(), 1u);
}

TEST(ActorTree, IndexOfNamesDrawOrderPosition) {
  StubActor root{Rect{0, 0, 100, 100}};
  Actor& first = root.add(std::make_unique<StubActor>(Rect{0, 0, 10, 10}));
  Actor& middle = root.add(std::make_unique<StubActor>(Rect{1, 1, 10, 10}));
  Actor& last = root.add(std::make_unique<StubActor>(Rect{2, 2, 10, 10}));
  StubActor stranger{Rect{0, 0, 10, 10}};

  EXPECT_EQ(root.indexOf(first), 0u);
  EXPECT_EQ(root.indexOf(middle), 1u);
  EXPECT_EQ(root.indexOf(last), 2u);
  EXPECT_EQ(root.indexOf(stranger), root.children().size());
}

TEST(ActorTree, InsertRestoresDrawOrderOfAMiddleChild) {
  StubActor root{Rect{0, 0, 100, 100}};
  root.add(std::make_unique<StubActor>(Rect{0, 0, 10, 10}));
  Actor& middle = root.add(std::make_unique<StubActor>(Rect{1, 1, 10, 10}));
  root.add(std::make_unique<StubActor>(Rect{2, 2, 10, 10}));

  const std::size_t index = root.indexOf(middle);
  Actor& back = root.insert(index, root.detach(middle));

  EXPECT_EQ(&back, &middle);
  EXPECT_EQ(middle.parent(), &root);
  EXPECT_EQ(root.indexOf(middle), 1u);

  // Draw order follows children_ order: the middle child paints second.
  const EditorContext ctx;
  RecordingRenderer r;
  const Viewport viewport;
  {
    const Subview view{viewport, Rect{0, 0, 1000, 1000}, r};
    root.draw(view, ctx);
  }
  const auto fills = fillsOfSize(r.calls, 10, 10);
  ASSERT_EQ(fills.size(), 3u);
  EXPECT_EQ(fills[1].x, 1);
}

TEST(ActorTree, InsertPastTheEndAppends) {
  StubActor root{Rect{0, 0, 100, 100}};
  Actor& only = root.add(std::make_unique<StubActor>(Rect{0, 0, 10, 10}));
  Actor& added = root.insert(99, std::make_unique<StubActor>(Rect{1, 1, 10, 10}));

  EXPECT_EQ(root.indexOf(only), 0u);
  EXPECT_EQ(root.indexOf(added), 1u);
}
