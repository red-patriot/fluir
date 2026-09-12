#include "editor/actors/scene.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
#include "compiler/utility/context.hpp"
#include "editor/actors/actor.hpp"
#include "editor/actors/function_decl_actor.hpp"
#include "editor/actors/node_actor.hpp"
#include "editor/actors/node_actors.hpp"
#include "editor/actors/rail_actors.hpp"
#include "editor/core/collecting_sink.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/layer.hpp"
#include "editor/core/loader.hpp"
#include "fixture_loader.hpp"
#include "recording_renderer.hpp"

// These tests assert *scene construction and hit-testing*: absolute actor
// bounds for a real fixture, topmost-first overlap resolution, out-of-bounds
// misses, rebuild clearing stale actors, and the construction-time factory
// picking the right concrete Actor subclass per pt::Node alternative.

namespace {

  using testutil::Loaded;
  using testutil::loadFixture;

  namespace fs = std::filesystem;

  using fluir::FlowGraphLocation;
  using fluir::ID;
  using fluir::Operator;
  using fluir::editor::Actor;
  using fluir::editor::BinaryActor;
  using fluir::editor::CallActor;
  using fluir::editor::ConduitActor;
  using fluir::editor::ConstantActor;
  using fluir::editor::EditorContext;
  using fluir::editor::FunctionDeclActor;
  using fluir::editor::GraphScene;
  using fluir::editor::Layer;
  using fluir::editor::NodeActor;
  using fluir::editor::ParameterActor;
  using fluir::editor::Rect;
  using fluir::editor::ReturnActor;
  using fluir::editor::UnaryActor;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::countOf;
  using testutil::DrawCall;
  using testutil::expectRectNear;
  using testutil::RecordingRenderer;

  const EditorContext kCtx;

  // Bare-bones FunctionDecl wrapping `body`, at unit location (0,0,0,width,height).
  fluir::pt::FunctionDecl makeFunction(ID id, int width, int height, fluir::pt::Block body) {
    fluir::pt::FunctionDecl fn;
    fn.id = id;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = width, .height = height};
    fn.name = "f";
    fn.body = std::move(body);
    return fn;
  }

  fluir::pt::ParseTree singleFunctionTree(fluir::pt::FunctionDecl fn) {
    fluir::pt::ParseTree tree;
    tree.declarations.emplace(fn.id, fluir::pt::Declaration{fn});
    return tree;
  }

  fluir::pt::ParseTree twoFunctionTree(fluir::pt::FunctionDecl a, fluir::pt::FunctionDecl b) {
    fluir::pt::ParseTree tree;
    tree.declarations.emplace(a.id, fluir::pt::Declaration{a});
    tree.declarations.emplace(b.id, fluir::pt::Declaration{b});
    return tree;
  }

  fluir::pt::Constant makeConstant(ID id, int x, int y, int z, int w, int h) {
    fluir::pt::Constant constant;
    constant.id = id;
    constant.location = FlowGraphLocation{.x = x, .y = y, .z = z, .width = w, .height = h};
    constant.value = fluir::literals_types::I32{0};
    return constant;
  }

  fluir::pt::Binary makeBinary(ID id, int x, int y, int z, int w, int h) {
    fluir::pt::Binary binary;
    binary.id = id;
    binary.location = FlowGraphLocation{.x = x, .y = y, .z = z, .width = w, .height = h};
    binary.lhs = 0;
    binary.rhs = 0;
    binary.op = Operator::PLUS;
    return binary;
  }

  fluir::pt::Unary makeUnary(ID id, int x, int y, int z, int w, int h) {
    fluir::pt::Unary unary;
    unary.id = id;
    unary.location = FlowGraphLocation{.x = x, .y = y, .z = z, .width = w, .height = h};
    unary.lhs = 0;
    unary.op = Operator::MINUS;
    return unary;
  }

  fluir::pt::Call makeCall(ID id, int x, int y, int z, int w, int h) {
    fluir::pt::Call call;
    call.id = id;
    call.location = FlowGraphLocation{.x = x, .y = y, .z = z, .width = w, .height = h};
    call.target = "add";
    return call;
  }

  fluir::pt::Conduit makeConduit(ID id, ID input, std::vector<fluir::pt::Conduit::Output> children) {
    fluir::pt::Conduit conduit;
    conduit.id = id;
    conduit.input = input;
    conduit.index = 0;
    conduit.children = std::move(children);
    return conduit;
  }

  // Draws `scene` through one Layer, the same path ModulePage uses.
  void drawScene(const GraphScene& scene, RecordingRenderer& r) {
    Layer layer;
    layer.setRoot(scene.root());
    layer.setViewport(Viewport{});
    layer.draw(r, kCtx, Rect{0, 0, r.outputSize().x, r.outputSize().y});
  }

  // Every ConduitActor hanging off function `functionId`'s body.
  std::size_t conduitCount(const GraphScene& scene, ID functionId) {
    const auto* frame = dynamic_cast<const FunctionDeclActor*>(scene.find(functionId));
    if (frame == nullptr) {
      return 0;
    }
    std::size_t count = 0;
    for (const auto& child : frame->body().children()) {
      if (dynamic_cast<const ConduitActor*>(child.get()) != nullptr) {
        ++count;
      }
    }
    return count;
  }

}  // namespace

TEST(Scene, BuildResolvesFixtureConstantToVerifiedAbsoluteRect) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  // int_constants.fl constant id=1: FlowGraphLocation x=2,y=20,w=5,h=5;
  // function main x=10,y=10,z=3,w=100,h=100; unitPx=5 -> functionOrigin
  // {50,50}, bodyOrigin {50,75}, localRect {10,100,25,25} -> absolute
  // {60,175,25,25}. Same value asserted in graph_draw.test.cpp and
  // graph_geometry.test.cpp.
  Actor* hit = scene.topmostAt(Vec2{65, 180});
  ASSERT_NE(hit, nullptr);
  EXPECT_EQ(hit->worldBounds(), (Rect{60, 175, 25, 25}));
  EXPECT_NE(dynamic_cast<ConstantActor*>(hit), nullptr);
}

TEST(Scene, TopmostAtReturnsHigherZOnOverlap) {
  // Two constants at the same absolute rect, differing only by z (and hence
  // paint order via sortedNodes): id=11 (z=5) paints after id=10 (z=1), so it
  // must win the hit test for a worldPos inside both.
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10, 1, 1, 1, 2, 2));
  body.nodes.emplace(11, makeConstant(11, 1, 1, 5, 2, 2));

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(makeFunction(1, 100, 100, std::move(body))));

  // localRect{5,5,10,10} -> bodyOrigin{0,25} -> absolute {5,30,10,10}.
  Actor* hit = scene.topmostAt(Vec2{8, 33});
  ASSERT_NE(hit, nullptr);
  EXPECT_EQ(dynamic_cast<NodeActor*>(hit)->id(), (fluir::FullID{1, 11}));
}

TEST(Scene, TopmostAtOutsideEveryActorReturnsNullptr) {
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10, 1, 1, 1, 2, 2));

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(makeFunction(1, 100, 100, std::move(body))));

  EXPECT_EQ(scene.topmostAt(Vec2{-500, -500}), nullptr);
}

TEST(Scene, RebuildClearsStaleActors) {
  fluir::pt::Block bodyA;
  bodyA.nodes.emplace(20, makeConstant(20, 1, 1, 1, 2, 2));
  // localRect{5,5,10,10} -> absolute {5,30,10,10}, center-ish {10,35}.

  fluir::pt::Block bodyB;
  bodyB.nodes.emplace(21, makeConstant(21, 10, 10, 1, 2, 2));
  // localRect{50,50,10,10} -> absolute {50,75,10,10}, center-ish {55,80}.

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(makeFunction(1, 200, 200, std::move(bodyA))));
  Actor* before = scene.topmostAt(Vec2{10, 35});
  ASSERT_NE(before, nullptr);
  EXPECT_NE(dynamic_cast<ConstantActor*>(before), nullptr);

  scene.build(kCtx, singleFunctionTree(makeFunction(1, 200, 200, std::move(bodyB))));

  // Stale actor A is gone: that point now falls through to the enclosing FunctionDeclActor
  // (still non-null -- it's frame area, not empty space), never to a ConstantActor.
  Actor* after = scene.topmostAt(Vec2{10, 35});
  ASSERT_NE(after, nullptr);
  EXPECT_EQ(dynamic_cast<ConstantActor*>(after), nullptr);

  Actor* hit = scene.topmostAt(Vec2{55, 80});
  ASSERT_NE(hit, nullptr);
  EXPECT_EQ(dynamic_cast<NodeActor*>(hit)->id(), (fluir::FullID{1, 21}));
}

TEST(Scene, FindReturnsActorOwningNodeId) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  // Fixture: function id=1, constant id=1.
  Actor* found = scene.find(1, 1);
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(dynamic_cast<NodeActor*>(found)->id(), (fluir::FullID{1, 1}));
  EXPECT_NE(dynamic_cast<ConstantActor*>(found), nullptr);
}

TEST(Scene, FindReturnsNullptrForUnknownId) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  EXPECT_EQ(scene.find(1, 999999), nullptr);
  EXPECT_EQ(scene.find(999999), nullptr);
}

TEST(Scene, FindReflectsRebuild) {
  fluir::pt::Block bodyA;
  bodyA.nodes.emplace(20, makeConstant(20, 1, 1, 1, 2, 2));

  fluir::pt::Block bodyB;
  bodyB.nodes.emplace(21, makeConstant(21, 10, 10, 1, 2, 2));

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(makeFunction(1, 200, 200, std::move(bodyA))));
  ASSERT_NE(scene.find(1, 20), nullptr);

  scene.build(kCtx, singleFunctionTree(makeFunction(1, 200, 200, std::move(bodyB))));

  EXPECT_EQ(scene.find(1, 20), nullptr);  // stale actor A must be gone
  EXPECT_NE(scene.find(1, 21), nullptr);
}

TEST(Scene, FindDisambiguatesFunctionFrameFromSameIdNode) {
  // Fixture: function id=1, constant id=1 -- the classic function-vs-node collision this
  // pass exists to fix. find(1) must hit the frame; find(1, 1) must hit the node.
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  Actor* frame = scene.find(1);
  Actor* node = scene.find(1, 1);
  ASSERT_NE(frame, nullptr);
  ASSERT_NE(node, nullptr);
  EXPECT_NE(frame, node);
  EXPECT_EQ(dynamic_cast<FunctionDeclActor*>(frame)->functionId(), 1u);
  EXPECT_EQ(dynamic_cast<NodeActor*>(node)->id(), (fluir::FullID{1, 1}));
  EXPECT_NE(dynamic_cast<FunctionDeclActor*>(frame), nullptr);
  EXPECT_NE(dynamic_cast<ConstantActor*>(node), nullptr);
}

TEST(Scene, FindDisambiguatesCrossFunctionNodeIdCollision) {
  // Two functions each declare a node with the same bare id (matches next_id.py's per-function
  // numbering restart): find(functionId, nodeId) must resolve to the right function's node.
  fluir::pt::Block bodyA;
  bodyA.nodes.emplace(2, makeConstant(2, 1, 1, 1, 2, 2));

  fluir::pt::Block bodyB;
  bodyB.nodes.emplace(2, makeConstant(2, 1, 1, 1, 2, 2));

  GraphScene scene;
  scene.build(
    kCtx, twoFunctionTree(makeFunction(1, 100, 100, std::move(bodyA)), makeFunction(2, 100, 100, std::move(bodyB))));

  Actor* nodeInA = scene.find(1, 2);
  Actor* nodeInB = scene.find(2, 2);
  ASSERT_NE(nodeInA, nullptr);
  ASSERT_NE(nodeInB, nullptr);
  EXPECT_NE(nodeInA, nodeInB);
  EXPECT_EQ(dynamic_cast<NodeActor*>(nodeInA)->id(), (fluir::FullID{1, 2}));
  EXPECT_EQ(dynamic_cast<NodeActor*>(nodeInB)->id(), (fluir::FullID{2, 2}));
}

TEST(Scene, FactoryConstructsCorrectConcreteTypePerVariant) {
  fluir::pt::Block body;
  body.nodes.emplace(30, makeBinary(30, 1, 1, 1, 2, 2));     // localRect {5,5,10,10}   -> abs {5,30,10,10}
  body.nodes.emplace(31, makeUnary(31, 10, 1, 1, 2, 2));     // localRect {50,5,10,10}  -> abs {50,30,10,10}
  body.nodes.emplace(32, makeConstant(32, 20, 1, 1, 2, 2));  // localRect {100,5,10,10} -> abs {100,30,10,10}
  body.nodes.emplace(33, makeCall(33, 30, 1, 1, 2, 2));      // localRect {150,5,10,10} -> abs {150,30,10,10}

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(makeFunction(1, 300, 100, std::move(body))));

  Actor* binary = scene.topmostAt(Vec2{8, 33});
  Actor* unary = scene.topmostAt(Vec2{53, 33});
  Actor* constant = scene.topmostAt(Vec2{103, 33});
  Actor* call = scene.topmostAt(Vec2{153, 33});

  ASSERT_NE(binary, nullptr);
  ASSERT_NE(unary, nullptr);
  ASSERT_NE(constant, nullptr);
  ASSERT_NE(call, nullptr);

  EXPECT_NE(dynamic_cast<BinaryActor*>(binary), nullptr);
  EXPECT_NE(dynamic_cast<UnaryActor*>(unary), nullptr);
  EXPECT_NE(dynamic_cast<ConstantActor*>(constant), nullptr);
  EXPECT_NE(dynamic_cast<CallActor*>(call), nullptr);

  // Cross-check: no accidental type crossover (e.g. binary slot holding a UnaryActor).
  EXPECT_EQ(dynamic_cast<UnaryActor*>(binary), nullptr);
  EXPECT_EQ(dynamic_cast<ConstantActor*>(binary), nullptr);
  EXPECT_EQ(dynamic_cast<CallActor*>(binary), nullptr);
}

TEST(Scene, TopmostAtPrefersNodeOverEnclosingFrameOnOverlap) {
  // FunctionDeclActor is pushed before its function's node actors, so on overlap the
  // (later-painted) node actor must win the reverse-order hit test.
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10, 1, 1, 1, 2, 2));  // localRect{5,5,10,10} -> abs {5,30,10,10}

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(makeFunction(1, 100, 100, std::move(body))));

  Actor* hit = scene.topmostAt(Vec2{8, 33});
  ASSERT_NE(hit, nullptr);
  EXPECT_NE(dynamic_cast<ConstantActor*>(hit), nullptr);
}

TEST(Scene, TopmostAtFallsBackToFrameOverEmptyFrameArea) {
  // A click inside the frame but outside every node's bounds must hit the frame chrome
  // itself, not fall through to nullptr.
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10, 1, 1, 1, 2, 2));  // localRect{5,5,10,10} -> abs {5,30,10,10}

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(makeFunction(1, 100, 100, std::move(body))));

  // Frame spans absolute {0,0,500,500}; (400,400) is inside the frame but far from the node.
  Actor* hit = scene.topmostAt(Vec2{400, 400});
  ASSERT_NE(hit, nullptr);
  EXPECT_NE(dynamic_cast<FunctionDeclActor*>(hit), nullptr);
  EXPECT_EQ(dynamic_cast<FunctionDeclActor*>(hit)->functionId(), 1u);
}

// GraphScene::worldBounds() is the fit-to-view source: it must read live actor
// bounds, so a drag is reflected without a rebuild or a save.

TEST(SceneBounds, EmptyTreeIsZero) {
  const Loaded l = loadFixture("read/top_level_comment_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);
  expectRectNear(scene.worldBounds(), Rect{0, 0, 0, 0});
}

TEST(SceneBounds, SingleFunctionIsItsFrame) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  // Fixture: x=10 y=10 w=100 h=100 (logical); world px = logical * UNIT_PX (5).
  GraphScene scene;
  scene.build(kCtx, *l.result.tree);
  expectRectNear(scene.worldBounds(), Rect{50, 50, 500, 500});
}

TEST(SceneBounds, MultipleFunctionsUnion) {
  const Loaded l = loadFixture("read/multiple_empty_functions.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  // Fixture frames (logical -> world px, * UNIT_PX == 5):
  //   foo x=10  y=10 w=100 h=100 -> world [ 50, 50 ..  550, 550]
  //   baz x=330 y=10 w=100 h=100 -> world [1650, 50 .. 2150, 550]
  //   bar x=210 y=10 w=50  h=70  -> world [1050, 50 .. 1300, 400]
  // union: min (50, 50), max (2150, 550) -> {50, 50, 2100, 500}.
  GraphScene scene;
  scene.build(kCtx, *l.result.tree);
  expectRectNear(scene.worldBounds(), Rect{50, 50, 2100, 500});
}

TEST(SceneBounds, ScalesWithUnitPx) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  // Fixture: x=10 y=10 w=100 h=100 (logical). With unitPx = 10, world px doubles
  // relative to the default (5): {100, 100, 1000, 1000}.
  EditorContext ctx;
  ctx.layout.unitPx = 10.0;
  GraphScene scene;
  scene.build(ctx, *l.result.tree);
  expectRectNear(scene.worldBounds(), Rect{100, 100, 1000, 1000});
}

TEST(SceneBounds, ReflectsAFrameDragWithoutASave) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  // Frame world box {50,50,500,500}; its drag handle sits at the header's right
  // edge, {530,55,15,15}. Drag it 20 logical units right and 10 down.
  auto* frame = dynamic_cast<FunctionDeclActor*>(scene.find(1));
  ASSERT_NE(frame, nullptr);
  ASSERT_TRUE(frame->gestures()->press(kCtx, Vec2{537.5, 62.5}));
  frame->gestures()->drag(kCtx, Vec2{20 * kCtx.layout.unitPx, 10 * kCtx.layout.unitPx});
  scene.layout(kCtx);

  expectRectNear(scene.worldBounds(), Rect{150, 100, 500, 500});
}

// Conduits resolve their endpoints by id at layout time, so an endpoint the file
// leaves dangling is a drawing gap, never a dropped conduit.

TEST(Scene, ConduitWithUnresolvedSourceDrawsNoLine) {
  fluir::pt::Block body;
  body.nodes.emplace(30, makeBinary(30, 10, 1, 1, 5, 5));
  body.conduits.emplace(40, makeConduit(40, 999, {{.target = 30, .index = 0}}));

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(makeFunction(1, 100, 100, std::move(body))));

  EXPECT_EQ(conduitCount(scene, 1), 1u);

  RecordingRenderer r;
  drawScene(scene, r);
  EXPECT_EQ(countOf(r.calls, DrawCall::Op::Line), 0u);
}

TEST(Scene, ConduitDrawsOnlyItsResolvableTargets) {
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10, 1, 1, 1, 5, 5));
  body.nodes.emplace(30, makeBinary(30, 10, 1, 1, 5, 5));
  body.conduits.emplace(40, makeConduit(40, 10, {{.target = 30, .index = 0}, {.target = 999, .index = 0}}));

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(makeFunction(1, 100, 100, std::move(body))));

  EXPECT_EQ(conduitCount(scene, 1), 1u);

  RecordingRenderer r;
  drawScene(scene, r);
  EXPECT_EQ(countOf(r.calls, DrawCall::Op::Line), 1u);
}

// The actor tree must retain everything the writer needs, so the parse tree can
// be regenerated from it rather than kept alive alongside it.

TEST(SceneRetention, NodeActorsReturnTheirParseTreeNode) {
  const fluir::pt::Binary binary = makeBinary(30, 1, 1, 1, 2, 2);
  const fluir::pt::Unary unary = makeUnary(31, 10, 1, 1, 2, 2);
  const fluir::pt::Constant constant = makeConstant(32, 20, 1, 1, 2, 2);
  fluir::pt::Call call = makeCall(33, 30, 1, 1, 2, 2);
  call._return = fluir::pt::Call::Return{};
  call.arguments = {{.name = "a", .index = 0}, {.name = "b", .index = 1}};

  fluir::pt::Block body;
  body.nodes.emplace(30, binary);
  body.nodes.emplace(31, unary);
  body.nodes.emplace(32, constant);
  body.nodes.emplace(33, call);

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(makeFunction(1, 300, 100, std::move(body))));

  EXPECT_EQ(dynamic_cast<NodeActor*>(scene.find(1, 30))->node(), fluir::pt::Node{binary});
  EXPECT_EQ(dynamic_cast<NodeActor*>(scene.find(1, 31))->node(), fluir::pt::Node{unary});
  EXPECT_EQ(dynamic_cast<NodeActor*>(scene.find(1, 32))->node(), fluir::pt::Node{constant});
  EXPECT_EQ(dynamic_cast<NodeActor*>(scene.find(1, 33))->node(), fluir::pt::Node{call});
}

TEST(SceneRetention, RailActorsKeepNameAndTypeApart) {
  fluir::pt::FunctionDecl fn = makeFunction(1, 100, 100, fluir::pt::Block{});
  fn.input = fluir::pt::FunctionDecl::InputBlock{{{.id = 5, .index = 0, .name = "lhs", .typeName = "i32"}}};
  fn.output = fluir::pt::FunctionDecl::OutputBlock{fluir::pt::FunctionDecl::Return{.id = 6, .typeName = "i32"}};

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(fn));

  const auto* frame = dynamic_cast<const FunctionDeclActor*>(scene.find(1));
  ASSERT_NE(frame, nullptr);
  EXPECT_EQ(frame->name(), "f");

  const auto* param = dynamic_cast<const ParameterActor*>(frame->port(5));
  ASSERT_NE(param, nullptr);
  EXPECT_EQ(param->parameter(), fn.input->parameters.front());

  const auto* ret = dynamic_cast<const ReturnActor*>(frame->port(6));
  ASSERT_NE(ret, nullptr);
  EXPECT_EQ(ret->ret(), *fn.output->ret);
}

TEST(SceneRetention, ConduitActorIsNamedByItsIdAndRetainsItsConduit) {
  const fluir::pt::Conduit conduit = makeConduit(40, 10, {{.target = 30, .index = 1}});

  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10, 1, 1, 1, 5, 5));
  body.nodes.emplace(30, makeBinary(30, 10, 1, 1, 5, 5));
  body.conduits.emplace(40, conduit);

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(makeFunction(1, 100, 100, std::move(body))));

  const auto* actor = dynamic_cast<const ConduitActor*>(scene.find(1, 40));
  ASSERT_NE(actor, nullptr);
  EXPECT_EQ(actor->selectionId(), (fluir::FullID{1, 40}));
  EXPECT_EQ(actor->conduit(), conduit);
}

// detach/attach are the scene's undo primitives: they must keep byId_, the
// frame registry and draw order consistent, not just the actor tree.

TEST(SceneDetach, DetachRemovesTheActorFromLookupAndFromTheDrawing) {
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10, 1, 1, 1, 5, 5));
  body.nodes.emplace(11, makeConstant(11, 10, 1, 1, 5, 5));

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(makeFunction(1, 100, 100, std::move(body))));

  RecordingRenderer before;
  drawScene(scene, before);
  const std::size_t fillsBefore = testutil::fillsOf(before.calls).size();

  GraphScene::DetachedActor detached = scene.detach(fluir::FullID{1, 10});
  ASSERT_NE(detached.actor, nullptr);
  EXPECT_EQ(scene.find(1, 10), nullptr);

  scene.layout(kCtx);
  RecordingRenderer after;
  drawScene(scene, after);
  EXPECT_LT(testutil::fillsOf(after.calls).size(), fillsBefore);
}

TEST(SceneDetach, AttachRestoresLookupAndDrawOrder) {
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10, 1, 1, 1, 5, 5));
  body.nodes.emplace(11, makeConstant(11, 10, 1, 1, 5, 5));
  body.nodes.emplace(12, makeConstant(12, 20, 1, 1, 5, 5));

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(makeFunction(1, 100, 100, std::move(body))));

  RecordingRenderer before;
  drawScene(scene, before);

  GraphScene::DetachedActor detached = scene.detach(fluir::FullID{1, 11});
  ASSERT_NE(detached.actor, nullptr);
  EXPECT_TRUE(scene.attach(std::move(detached)));

  EXPECT_NE(scene.find(1, 11), nullptr);

  scene.layout(kCtx);
  RecordingRenderer after;
  drawScene(scene, after);
  EXPECT_EQ(after.calls, before.calls);  // same primitives, same order
}

TEST(SceneDetach, DetachingAnUnknownIdYieldsNothing) {
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10, 1, 1, 1, 5, 5));

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(makeFunction(1, 100, 100, std::move(body))));

  EXPECT_EQ(scene.detach(fluir::FullID{1, 999}).actor, nullptr);
  EXPECT_EQ(scene.detach(fluir::FullID{999}).actor, nullptr);
  EXPECT_NE(scene.find(1, 10), nullptr);
}

TEST(SceneDetach, DetachingAFrameTakesItsBodyWithIt) {
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10, 1, 1, 1, 5, 5));

  GraphScene scene;
  scene.build(kCtx, singleFunctionTree(makeFunction(1, 100, 100, std::move(body))));

  GraphScene::DetachedActor detached = scene.detach(fluir::FullID{1});
  ASSERT_NE(detached.actor, nullptr);
  EXPECT_EQ(scene.find(1), nullptr);
  EXPECT_EQ(scene.find(1, 10), nullptr);

  scene.layout(kCtx);
  RecordingRenderer after;
  drawScene(scene, after);
  EXPECT_TRUE(testutil::fillsOf(after.calls).empty());

  EXPECT_TRUE(scene.attach(std::move(detached)));
  EXPECT_NE(scene.find(1), nullptr);
  EXPECT_NE(scene.find(1, 10), nullptr);
}
