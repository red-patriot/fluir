#include "editor/transaction/transaction.hpp"

#include <cstddef>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
#include "editor/actors/scene.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/layer.hpp"
#include "editor/core/scene_to_tree.hpp"
#include "editor/transaction/delete.hpp"
#include "editor/transaction/move.hpp"
#include "editor/transaction/resize.hpp"
#include "recording_renderer.hpp"

// The house round trip: apply, assert, reverse, assert the scene is back --
// asserted through sceneToParseTree, a complete description of the scene.

namespace {

  using fluir::FlowGraphLocation;
  using fluir::ID;
  using fluir::Operator;
  using fluir::editor::DeleteTransaction;
  using fluir::editor::EditorContext;
  using fluir::editor::GraphScene;
  using fluir::editor::Layer;
  using fluir::editor::MoveTransaction;
  using fluir::editor::Rect;
  using fluir::editor::ResizeTransaction;
  using fluir::editor::sceneToParseTree;
  using fluir::editor::Viewport;
  using testutil::RecordingRenderer;

  const EditorContext kCtx;
  const fluir::pt::Header kHeader{.version = {0, 1, 3}};

  fluir::pt::Constant makeConstant(ID id, int x, int y) {
    fluir::pt::Constant constant;
    constant.id = id;
    constant.location = FlowGraphLocation{.x = x, .y = y, .z = 1, .width = 5, .height = 5};
    constant.value = fluir::literals_types::I32{0};
    return constant;
  }

  fluir::pt::Binary makeBinary(ID id, int x, int y, ID lhs, ID rhs) {
    fluir::pt::Binary binary;
    binary.id = id;
    binary.location = FlowGraphLocation{.x = x, .y = y, .z = 1, .width = 5, .height = 5};
    binary.lhs = lhs;
    binary.rhs = rhs;
    binary.op = Operator::PLUS;
    return binary;
  }

  fluir::pt::Unary makeUnary(ID id, int x, int y, ID lhs) {
    fluir::pt::Unary unary;
    unary.id = id;
    unary.location = FlowGraphLocation{.x = x, .y = y, .z = 1, .width = 5, .height = 5};
    unary.lhs = lhs;
    unary.op = Operator::MINUS;
    return unary;
  }

  fluir::pt::Conduit makeConduit(ID id, ID input, ID target, int index) {
    fluir::pt::Conduit conduit;
    conduit.id = id;
    conduit.input = input;
    conduit.children.push_back({.target = target, .index = index});
    return conduit;
  }

  // One function: two constants feeding a binary, whose result feeds a unary.
  // Conduit 42 touches neither constant 10 nor binary 30, so it is the control.
  fluir::pt::ParseTree makeTree() {
    fluir::pt::FunctionDecl fn;
    fn.id = 1;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    fn.body.nodes.emplace(10, makeConstant(10, 1, 1));
    fn.body.nodes.emplace(11, makeConstant(11, 1, 10));
    fn.body.nodes.emplace(30, makeBinary(30, 10, 1, 10, 11));
    fn.body.nodes.emplace(31, makeUnary(31, 20, 1, 30));
    fn.body.conduits.emplace(40, makeConduit(40, 10, 30, 0));
    fn.body.conduits.emplace(41, makeConduit(41, 11, 30, 1));
    fn.body.conduits.emplace(42, makeConduit(42, 11, 31, 0));
    fn.body.conduits.emplace(43, makeConduit(43, 30, 31, 0));

    fluir::pt::ParseTree tree;
    tree.header = kHeader;
    tree.declarations.emplace(fn.id, fluir::pt::Declaration{fn});
    return tree;
  }

  std::vector<testutil::DrawCall> drawCalls(const GraphScene& scene) {
    RecordingRenderer r;
    Layer layer;
    layer.setRoot(scene.root());
    layer.setViewport(Viewport{});
    layer.draw(r, kCtx, Rect{0, 0, r.outputSize().x, r.outputSize().y});
    return r.calls;
  }

}  // namespace

TEST(MoveTransaction, RoundTripRestoresTheScene) {
  GraphScene scene;
  scene.build(kCtx, makeTree());
  const fluir::pt::ParseTree before = sceneToParseTree(scene, kHeader);

  MoveTransaction uut{fluir::FullID{1, 10}, 7, 9};
  ASSERT_TRUE(uut.execute(scene));
  EXPECT_NE(sceneToParseTree(scene, kHeader), before);
  ASSERT_TRUE(uut.unexecute(scene));
  EXPECT_EQ(sceneToParseTree(scene, kHeader), before);
}

TEST(MoveTransaction, RedoReachesTheSameStateAsTheFirstExecute) {
  GraphScene scene;
  scene.build(kCtx, makeTree());

  MoveTransaction uut{fluir::FullID{1, 10}, 7, 9};
  ASSERT_TRUE(uut.execute(scene));
  const fluir::pt::ParseTree afterFirst = sceneToParseTree(scene, kHeader);
  ASSERT_TRUE(uut.unexecute(scene));
  ASSERT_TRUE(uut.execute(scene));

  EXPECT_EQ(sceneToParseTree(scene, kHeader), afterFirst);
}

TEST(MoveTransaction, MissChangesNothingAndReturnsFalse) {
  GraphScene scene;
  scene.build(kCtx, makeTree());
  const fluir::pt::ParseTree before = sceneToParseTree(scene, kHeader);

  MoveTransaction unknown{fluir::FullID{1, 999}, 7, 9};
  EXPECT_FALSE(unknown.execute(scene));

  // A drag that ended where it started is a no-op swap, not an edit.
  MoveTransaction inPlace{fluir::FullID{1, 10}, 1, 1};
  EXPECT_FALSE(inPlace.execute(scene));

  EXPECT_EQ(sceneToParseTree(scene, kHeader), before);
}

TEST(ResizeTransaction, ResizeSetsTheWidthAndHeight) {
  GraphScene scene;
  scene.build(kCtx, makeTree());

  ResizeTransaction uut{fluir::FullID{1, 10}, 9, 7};
  ASSERT_TRUE(uut.execute(scene));

  const fluir::FlowGraphLocation* loc = scene.find(1, 10)->location();
  ASSERT_NE(loc, nullptr);
  EXPECT_EQ(loc->width, 9);
  EXPECT_EQ(loc->height, 7);
}

TEST(ResizeTransaction, ResizeUndoRestoresTheOriginalSize) {
  GraphScene scene;
  scene.build(kCtx, makeTree());
  const fluir::pt::ParseTree before = sceneToParseTree(scene, kHeader);

  ResizeTransaction uut{fluir::FullID{1, 10}, 9, 7};
  ASSERT_TRUE(uut.execute(scene));
  EXPECT_NE(sceneToParseTree(scene, kHeader), before);
  ASSERT_TRUE(uut.unexecute(scene));
  EXPECT_EQ(sceneToParseTree(scene, kHeader), before);
}

TEST(ResizeTransaction, ResizeClampsBelowTheMinimum) {
  GraphScene scene;
  scene.build(kCtx, makeTree());

  ResizeTransaction uut{fluir::FullID{1, 10}, -3, 0};
  ASSERT_TRUE(uut.execute(scene));

  const fluir::FlowGraphLocation* loc = scene.find(1, 10)->location();
  EXPECT_EQ(loc->width, fluir::editor::MIN_SIZE);
  EXPECT_EQ(loc->height, fluir::editor::MIN_SIZE);
}

TEST(ResizeTransaction, ResizeClampsAboveTheMaximum) {
  GraphScene scene;
  scene.build(kCtx, makeTree());

  ResizeTransaction uut{fluir::FullID{1, 10}, 10'000, 99'999};
  ASSERT_TRUE(uut.execute(scene));

  const fluir::FlowGraphLocation* loc = scene.find(1, 10)->location();
  EXPECT_EQ(loc->width, fluir::editor::MAX_SIZE);
  EXPECT_EQ(loc->height, fluir::editor::MAX_SIZE);
}

TEST(ResizeTransaction, ResizeToTheCurrentSizeChangesNothing) {
  GraphScene scene;
  scene.build(kCtx, makeTree());
  const fluir::pt::ParseTree before = sceneToParseTree(scene, kHeader);

  ResizeTransaction uut{fluir::FullID{1, 10}, 5, 5};
  EXPECT_FALSE(uut.execute(scene));
  EXPECT_EQ(sceneToParseTree(scene, kHeader), before);
}

TEST(ResizeTransaction, ResizeOfAnUnknownIdFails) {
  GraphScene scene;
  scene.build(kCtx, makeTree());
  const fluir::pt::ParseTree before = sceneToParseTree(scene, kHeader);

  ResizeTransaction unknown{fluir::FullID{1, 999}, 9, 7};
  EXPECT_FALSE(unknown.execute(scene));
  EXPECT_EQ(sceneToParseTree(scene, kHeader), before);
}

TEST(DeleteTransaction, RoundTripRestoresNodeConduitsOperandsAndDrawOrder) {
  GraphScene scene;
  scene.build(kCtx, makeTree());
  const fluir::pt::ParseTree before = sceneToParseTree(scene, kHeader);
  const auto callsBefore = drawCalls(scene);

  DeleteTransaction uut{fluir::FullID{1, 30}};
  ASSERT_TRUE(uut.execute(scene));

  const fluir::pt::ParseTree after = sceneToParseTree(scene, kHeader);
  const auto& body = std::get<fluir::pt::FunctionDecl>(after.declarations.at(1)).body;
  EXPECT_EQ(body.nodes.count(30), 0u);
  EXPECT_EQ(body.conduits.count(40), 0u);  // targeted the node
  EXPECT_EQ(body.conduits.count(41), 0u);  // targeted the node
  EXPECT_EQ(body.conduits.count(43), 0u);  // sourced from the node
  EXPECT_EQ(body.conduits.count(42), 1u);  // the control, untouched
  EXPECT_EQ(std::get<fluir::pt::Unary>(body.nodes.at(31)).lhs, fluir::INVALID_ID);

  ASSERT_TRUE(uut.unexecute(scene));
  EXPECT_EQ(sceneToParseTree(scene, kHeader), before);

  scene.layout(kCtx);
  EXPECT_EQ(drawCalls(scene), callsBefore);
}

TEST(DeleteTransaction, RedoOfANodeReachesTheSameStateAsTheFirstExecute) {
  GraphScene scene;
  scene.build(kCtx, makeTree());

  DeleteTransaction uut{fluir::FullID{1, 30}};
  ASSERT_TRUE(uut.execute(scene));
  const fluir::pt::ParseTree afterFirst = sceneToParseTree(scene, kHeader);
  ASSERT_TRUE(uut.unexecute(scene));
  ASSERT_TRUE(uut.execute(scene));

  EXPECT_EQ(sceneToParseTree(scene, kHeader), afterFirst);
}

TEST(DeleteTransaction, UnknownNodeIdChangesNothingAndReturnsFalse) {
  GraphScene scene;
  scene.build(kCtx, makeTree());
  const fluir::pt::ParseTree before = sceneToParseTree(scene, kHeader);

  DeleteTransaction uut{fluir::FullID{1, 999}};
  EXPECT_FALSE(uut.execute(scene));
  EXPECT_EQ(sceneToParseTree(scene, kHeader), before);
}

TEST(DeleteTransaction, RoundTripRestoresTheWholeFrame) {
  GraphScene scene;
  scene.build(kCtx, makeTree());
  const fluir::pt::ParseTree before = sceneToParseTree(scene, kHeader);
  const auto callsBefore = drawCalls(scene);

  DeleteTransaction uut{fluir::FullID{1}};
  ASSERT_TRUE(uut.execute(scene));
  EXPECT_TRUE(sceneToParseTree(scene, kHeader).declarations.empty());

  ASSERT_TRUE(uut.unexecute(scene));
  EXPECT_EQ(sceneToParseTree(scene, kHeader), before);

  scene.layout(kCtx);
  EXPECT_EQ(drawCalls(scene), callsBefore);
}

TEST(DeleteTransaction, RedoOfAFrameReachesTheSameStateAsTheFirstExecute) {
  GraphScene scene;
  scene.build(kCtx, makeTree());

  DeleteTransaction uut{fluir::FullID{1}};
  ASSERT_TRUE(uut.execute(scene));
  const fluir::pt::ParseTree afterFirst = sceneToParseTree(scene, kHeader);
  ASSERT_TRUE(uut.unexecute(scene));
  ASSERT_TRUE(uut.execute(scene));

  EXPECT_EQ(sceneToParseTree(scene, kHeader), afterFirst);
}

TEST(DeleteTransaction, UnknownFrameIdChangesNothingAndReturnsFalse) {
  GraphScene scene;
  scene.build(kCtx, makeTree());
  const fluir::pt::ParseTree before = sceneToParseTree(scene, kHeader);

  DeleteTransaction uut{fluir::FullID{999}};
  EXPECT_FALSE(uut.execute(scene));
  EXPECT_EQ(sceneToParseTree(scene, kHeader), before);
}

TEST(DeleteTransaction, EmptyIdChangesNothingAndReturnsFalse) {
  GraphScene scene;
  scene.build(kCtx, makeTree());
  const fluir::pt::ParseTree before = sceneToParseTree(scene, kHeader);

  DeleteTransaction uut{fluir::FullID{}};
  EXPECT_FALSE(uut.execute(scene));
  EXPECT_EQ(sceneToParseTree(scene, kHeader), before);
}

// A conduit id must not be mistaken for a node, which would drag its neighbours out.
TEST(DeleteTransaction, ConduitIdChangesNothingAndReturnsFalse) {
  GraphScene scene;
  scene.build(kCtx, makeTree());
  const fluir::pt::ParseTree before = sceneToParseTree(scene, kHeader);

  DeleteTransaction uut{fluir::FullID{1, 40}};
  EXPECT_FALSE(uut.execute(scene));
  EXPECT_EQ(sceneToParseTree(scene, kHeader), before);
}
