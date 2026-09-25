#include "editor/core/tree_edit.hpp"

#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
#include "editor/core/tree.hpp"

// Tree surgery, asserted against a hand-built et::ParseTree: no page, no
// renderer, no SDL. Delete is full referential cleanup -- conduits sourced from
// or targeting the node, plus Binary/Unary operand ids.

namespace {

  using fluir::FlowGraphLocation;
  using fluir::ID;
  using fluir::Operator;
  using fluir::editor::deleteNode;
  using fluir::editor::hasOperand;
  using fluir::editor::touches;

  fluir::editor::et::FunctionDecl makeFunction(ID id, fluir::editor::et::Block body) {
    fluir::editor::et::FunctionDecl fn;
    fn.id = id;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    fn.body = std::move(body);
    return fn;
  }

  fluir::editor::et::Constant makeConstant(ID id) {
    fluir::editor::et::Constant constant;
    constant.id = id;
    constant.location = FlowGraphLocation{.x = 1, .y = 1, .z = 1, .width = 2, .height = 2};
    constant.value = fluir::literals_types::I32{0};
    return constant;
  }

  fluir::editor::et::Binary makeBinary(ID id, ID lhs, ID rhs) {
    fluir::editor::et::Binary binary;
    binary.id = id;
    binary.location = FlowGraphLocation{.x = 1, .y = 1, .z = 1, .width = 2, .height = 2};
    binary.lhs = lhs;
    binary.rhs = rhs;
    binary.op = Operator::PLUS;
    return binary;
  }

  fluir::editor::et::Unary makeUnary(ID id, ID lhs) {
    fluir::editor::et::Unary unary;
    unary.id = id;
    unary.location = FlowGraphLocation{.x = 1, .y = 1, .z = 1, .width = 2, .height = 2};
    unary.lhs = lhs;
    unary.op = Operator::MINUS;
    return unary;
  }

  fluir::editor::et::Conduit makeConduit(ID id, ID input, std::vector<fluir::editor::et::Conduit::Output> children) {
    fluir::editor::et::Conduit conduit;
    conduit.id = id;
    conduit.input = input;
    conduit.children = std::move(children);
    return conduit;
  }

}  // namespace

TEST(TreeEdit, DeleteNodeRemovesItFromTheBody) {
  fluir::editor::et::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeConstant(11));
  fluir::editor::et::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 10));

  EXPECT_FALSE(fn.body.nodes.contains(10));
  EXPECT_TRUE(fn.body.nodes.contains(11));
}

TEST(TreeEdit, DeleteNodeReturnsFalseForUnknownIdAndLeavesTheBodyIntact) {
  fluir::editor::et::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.conduits.emplace(100, makeConduit(100, 10, {{.target = 11, .index = 0}}));
  fluir::editor::et::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_FALSE(deleteNode(fn.body, 999));

  EXPECT_EQ(fn.body.nodes.size(), 1u);
  EXPECT_EQ(fn.body.conduits.size(), 1u);
}

TEST(TreeEdit, DeleteNodeErasesConduitsSourcedFromIt) {
  fluir::editor::et::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeBinary(11, 10, 0));
  body.conduits.emplace(100, makeConduit(100, 10, {{.target = 11, .index = 0}}));
  fluir::editor::et::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 10));

  EXPECT_TRUE(fn.body.conduits.empty());
}

TEST(TreeEdit, DeleteNodeDropsOnlyTheMatchingTargetFromAMultiTargetConduit) {
  fluir::editor::et::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeBinary(11, 10, 0));
  body.nodes.emplace(12, makeBinary(12, 10, 0));
  body.conduits.emplace(100, makeConduit(100, 10, {{.target = 11, .index = 0}, {.target = 12, .index = 1}}));
  fluir::editor::et::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 11));

  ASSERT_TRUE(fn.body.conduits.contains(100));
  const auto& children = fn.body.conduits.at(100).children;
  ASSERT_EQ(children.size(), 1u);
  EXPECT_EQ(children[0].target, 12u);
  EXPECT_EQ(children[0].index, 1);  // the surviving target keeps its own index
}

TEST(TreeEdit, DeleteNodeErasesAConduitWhoseLastTargetItWas) {
  fluir::editor::et::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeBinary(11, 10, 0));
  body.conduits.emplace(100, makeConduit(100, 10, {{.target = 11, .index = 0}}));
  fluir::editor::et::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 11));

  EXPECT_TRUE(fn.body.conduits.empty());
  EXPECT_TRUE(fn.body.nodes.contains(10));
}

TEST(TreeEdit, DeleteNodeKeepsAnAlreadyChildlessConduit) {
  // A conduit with no targets is not collateral: only one that *loses* its last
  // target goes.
  fluir::editor::et::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeConstant(11));
  body.conduits.emplace(100, makeConduit(100, 10, {}));
  fluir::editor::et::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 11));

  EXPECT_TRUE(fn.body.conduits.contains(100));
}

TEST(TreeEdit, DeleteNodeResetsBinaryLhsThatReferencedIt) {
  fluir::editor::et::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeBinary(11, 10, 12));
  body.nodes.emplace(12, makeConstant(12));
  fluir::editor::et::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 10));

  const auto& binary = std::get<fluir::editor::et::Binary>(fn.body.nodes.at(11));
  EXPECT_EQ(binary.lhs, fluir::INVALID_ID);
  EXPECT_EQ(binary.rhs, 12u);  // the untouched operand is left alone
}

TEST(TreeEdit, DeleteNodeResetsBinaryRhsThatReferencedIt) {
  fluir::editor::et::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeBinary(11, 12, 10));
  body.nodes.emplace(12, makeConstant(12));
  fluir::editor::et::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 10));

  const auto& binary = std::get<fluir::editor::et::Binary>(fn.body.nodes.at(11));
  EXPECT_EQ(binary.rhs, fluir::INVALID_ID);
  EXPECT_EQ(binary.lhs, 12u);
}

TEST(TreeEdit, DeleteNodeResetsUnaryLhsThatReferencedIt) {
  fluir::editor::et::Block body;
  body.nodes.emplace(10, makeConstant(10));
  body.nodes.emplace(11, makeUnary(11, 10));
  fluir::editor::et::FunctionDecl fn = makeFunction(1, std::move(body));

  EXPECT_TRUE(deleteNode(fn.body, 10));

  EXPECT_EQ(std::get<fluir::editor::et::Unary>(fn.body.nodes.at(11)).lhs, fluir::INVALID_ID);
}

TEST(TreeEdit, DeleteNodeLeavesOtherFunctionsUntouched) {
  // Node ids are body-scoped: two functions may both hold id 10.
  fluir::editor::et::Block bodyA;
  bodyA.nodes.emplace(10, makeConstant(10));
  fluir::editor::et::Block bodyB;
  bodyB.nodes.emplace(10, makeConstant(10));
  bodyB.conduits.emplace(100, makeConduit(100, 10, {}));

  fluir::editor::et::FunctionDecl a = makeFunction(1, std::move(bodyA));
  fluir::editor::et::FunctionDecl b = makeFunction(2, std::move(bodyB));

  EXPECT_TRUE(deleteNode(a.body, 10));

  EXPECT_TRUE(a.body.nodes.empty());
  EXPECT_TRUE(b.body.nodes.contains(10));
  EXPECT_TRUE(b.body.conduits.contains(100));
}

TEST(TreeEdit, TouchesMatchesTheSourceOrAnyTarget) {
  const fluir::editor::et::Conduit conduit =
    makeConduit(100, 10, {{.target = 11, .index = 0}, {.target = 12, .index = 1}});

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
