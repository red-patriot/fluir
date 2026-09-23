#include "editor/core/tree_edit.hpp"

#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"

// Tree surgery, asserted against a hand-built pt::ParseTree: no page, no
// renderer, no SDL. Delete is full referential cleanup -- conduits sourced from
// or targeting the node, plus Binary/Unary operand ids.

namespace {

  using fluir::FlowGraphLocation;
  using fluir::ID;
  using fluir::Operator;
  using fluir::editor::deleteNode;
  using fluir::editor::hasOperand;
  using fluir::editor::touches;

  fluir::pt::FunctionDecl makeFunction(ID id, fluir::pt::Block body) {
    fluir::pt::FunctionDecl fn;
    fn.id = id;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    fn.body = std::move(body);
    return fn;
  }

  fluir::pt::Constant makeConstant(ID id) {
    fluir::pt::Constant constant;
    constant.id = id;
    constant.location = FlowGraphLocation{.x = 1, .y = 1, .z = 1, .width = 2, .height = 2};
    constant.value = fluir::literals_types::I32{0};
    return constant;
  }

  fluir::pt::Binary makeBinary(ID id, ID lhs, ID rhs) {
    fluir::pt::Binary binary;
    binary.id = id;
    binary.location = FlowGraphLocation{.x = 1, .y = 1, .z = 1, .width = 2, .height = 2};
    binary.lhs = lhs;
    binary.rhs = rhs;
    binary.op = Operator::PLUS;
    return binary;
  }

  fluir::pt::Unary makeUnary(ID id, ID lhs) {
    fluir::pt::Unary unary;
    unary.id = id;
    unary.location = FlowGraphLocation{.x = 1, .y = 1, .z = 1, .width = 2, .height = 2};
    unary.lhs = lhs;
    unary.op = Operator::MINUS;
    return unary;
  }

  fluir::pt::Conduit makeConduit(ID id, ID input, std::vector<fluir::pt::Conduit::Output> children) {
    fluir::pt::Conduit conduit;
    conduit.id = id;
    conduit.input = input;
    conduit.children = std::move(children);
    return conduit;
  }

}  // namespace

TEST(TreeEdit, DeleteNodeRemovesItFromTheBody) {
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeConstant(11));
  fluir::pt::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 10));

  EXPECT_FALSE(fn.body.nodes.contains(10));
  EXPECT_TRUE(fn.body.nodes.contains(11));
}

TEST(TreeEdit, DeleteNodeReturnsFalseForUnknownIdAndLeavesTheBodyIntact) {
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.conduits.emplace(100, makeConduit(100, 10, {{.target = 11, .index = 0}}));
  fluir::pt::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_FALSE(deleteNode(fn.body, 999));

  EXPECT_EQ(fn.body.nodes.size(), 1u);
  EXPECT_EQ(fn.body.conduits.size(), 1u);
}

TEST(TreeEdit, DeleteNodeErasesConduitsSourcedFromIt) {
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeBinary(11, 10, 0));
  body.conduits.emplace(100, makeConduit(100, 10, {{.target = 11, .index = 0}}));
  fluir::pt::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 10));

  EXPECT_TRUE(fn.body.conduits.empty());
}

TEST(TreeEdit, DeleteNodeDropsOnlyTheMatchingTargetFromAMultiTargetConduit) {
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeBinary(11, 10, 0));
  body.nodes.emplace(12, makeBinary(12, 10, 0));
  body.conduits.emplace(100, makeConduit(100, 10, {{.target = 11, .index = 0}, {.target = 12, .index = 1}}));
  fluir::pt::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 11));

  ASSERT_TRUE(fn.body.conduits.contains(100));
  const auto& children = fn.body.conduits.at(100).children;
  ASSERT_EQ(children.size(), 1u);
  EXPECT_EQ(children[0].target, 12u);
  EXPECT_EQ(children[0].index, 1);  // the surviving target keeps its own index
}

TEST(TreeEdit, DeleteNodeErasesAConduitWhoseLastTargetItWas) {
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeBinary(11, 10, 0));
  body.conduits.emplace(100, makeConduit(100, 10, {{.target = 11, .index = 0}}));
  fluir::pt::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 11));

  EXPECT_TRUE(fn.body.conduits.empty());
  EXPECT_TRUE(fn.body.nodes.contains(10));
}

TEST(TreeEdit, DeleteNodeKeepsAnAlreadyChildlessConduit) {
  // A conduit with no targets is not collateral: only one that *loses* its last
  // target goes.
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeConstant(11));
  body.conduits.emplace(100, makeConduit(100, 10, {}));
  fluir::pt::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 11));

  EXPECT_TRUE(fn.body.conduits.contains(100));
}

TEST(TreeEdit, DeleteNodeResetsBinaryLhsThatReferencedIt) {
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeBinary(11, 10, 12));
  body.nodes.emplace(12, makeConstant(12));
  fluir::pt::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 10));

  const auto& binary = std::get<fluir::pt::Binary>(fn.body.nodes.at(11));
  EXPECT_EQ(binary.lhs, fluir::INVALID_ID);
  EXPECT_EQ(binary.rhs, 12u);  // the untouched operand is left alone
}

TEST(TreeEdit, DeleteNodeResetsBinaryRhsThatReferencedIt) {
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeBinary(11, 12, 10));
  body.nodes.emplace(12, makeConstant(12));
  fluir::pt::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 10));

  const auto& binary = std::get<fluir::pt::Binary>(fn.body.nodes.at(11));
  EXPECT_EQ(binary.rhs, fluir::INVALID_ID);
  EXPECT_EQ(binary.lhs, 12u);
}

TEST(TreeEdit, DeleteNodeResetsUnaryLhsThatReferencedIt) {
  fluir::pt::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeUnary(11, 10));
  fluir::pt::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 10));

  EXPECT_EQ(std::get<fluir::pt::Unary>(fn.body.nodes.at(11)).lhs, fluir::INVALID_ID);
}

TEST(TreeEdit, DeleteNodeLeavesOtherFunctionsUntouched) {
  // Node ids are body-scoped: two functions may both hold id 10.
  fluir::pt::Block bodyA;
  bodyA.nodes.emplace(10, makeConstant(10));
  fluir::pt::Block bodyB;
  bodyB.nodes.emplace(10, makeConstant(10));
  bodyB.conduits.emplace(100, makeConduit(100, 10, {}));

  fluir::pt::FunctionDecl a = makeFunction(1, std::move(bodyA));
  fluir::pt::FunctionDecl b = makeFunction(2, std::move(bodyB));

  EXPECT_TRUE(deleteNode(a.body, 10));

  EXPECT_TRUE(a.body.nodes.empty());
  EXPECT_TRUE(b.body.nodes.contains(10));
  EXPECT_TRUE(b.body.conduits.contains(100));
}

TEST(TreeEdit, TouchesMatchesTheSourceOrAnyTarget) {
  const fluir::pt::Conduit conduit = makeConduit(100, 10, {{.target = 11, .index = 0}, {.target = 12, .index = 1}});

  EXPECT_TRUE(touches(conduit, 10));
  EXPECT_TRUE(touches(conduit, 12));
  EXPECT_FALSE(touches(conduit, 13));
}

TEST(TreeEdit, HasOperandMatchesBinaryAndUnaryOperandsOnly) {
  EXPECT_TRUE(hasOperand(makeBinary(11, 10, 12), 10));
  EXPECT_TRUE(hasOperand(makeBinary(11, 12, 10), 10));
  EXPECT_TRUE(hasOperand(makeUnary(11, 10), 10));
  EXPECT_FALSE(hasOperand(makeUnary(11, 12), 10));
  EXPECT_FALSE(hasOperand(makeConstant(10), 10));  // its own id is not an operand
}
