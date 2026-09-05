#include "editor/actors/scene.hpp"

#include <filesystem>
#include <string>

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
#include "editor/core/collecting_sink.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/loader.hpp"

// These tests assert *scene construction and hit-testing*: absolute actor
// bounds for a real fixture, topmost-first overlap resolution, out-of-bounds
// misses, rebuild clearing stale actors, and the construction-time factory
// picking the right concrete Actor subclass per pt::Node alternative.

namespace {

  namespace fs = std::filesystem;

  using fluir::FlowGraphLocation;
  using fluir::ID;
  using fluir::Operator;
  using fluir::editor::Actor;
  using fluir::editor::BinaryActor;
  using fluir::editor::CallActor;
  using fluir::editor::ConstantActor;
  using fluir::editor::EditorContext;
  using fluir::editor::FunctionDeclActor;
  using fluir::editor::GraphScene;
  using fluir::editor::NodeActor;
  using fluir::editor::Rect;
  using fluir::editor::UnaryActor;
  using fluir::editor::Vec2;

  const EditorContext kCtx;

  // Same fixture-loading shape as render_graph.test.cpp: each load owns its
  // Context + CollectingSink, version checks off (fixtures declare
  // <version>0.1.3</version>).
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

}  // namespace

TEST(Scene, BuildResolvesFixtureConstantToVerifiedAbsoluteRect) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  // int_constants.fl constant id=1: FlowGraphLocation x=2,y=20,w=5,h=5;
  // function main x=10,y=10,z=3,w=100,h=100; unitPx=5 -> functionOrigin
  // {50,50}, bodyOrigin {50,75}, localRect {10,100,25,25} -> absolute
  // {60,175,25,25}. Same value asserted in render_graph.test.cpp and
  // graph_geometry.test.cpp.
  Actor* hit = scene.topmostAt(Vec2{65, 180});
  ASSERT_NE(hit, nullptr);
  EXPECT_EQ(hit->bounds(), (Rect{60, 175, 25, 25}));
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
