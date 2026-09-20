#include "editor/core/tree_path.hpp"

#include <string>
#include <utility>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"

// Path resolution against a hand-built tree: a path names a function, or a
// node inside its parent's block. A path that resolves to nothing is nullptr.

namespace {

  using fluir::FlowGraphLocation;
  using fluir::FullID;
  using fluir::editor::blockOf;
  using fluir::editor::declarationAt;
  using fluir::editor::functionAt;
  using fluir::editor::isNodePath;
  using fluir::editor::isScopePath;
  using fluir::editor::locationAt;
  using fluir::editor::nodeAt;
  using fluir::editor::parentOf;
  using fluir::editor::railTypeAt;
  using fluir::editor::scopeAt;

  constexpr FlowGraphLocation kFnLoc{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
  constexpr FlowGraphLocation kNodeLoc{.x = 3, .y = 4, .z = 1, .width = 5, .height = 6};
  constexpr FlowGraphLocation kCommentLoc{.x = 7, .y = 8, .z = 2, .width = 9, .height = 10};

  // Function 1 holds constant 10 and conduit 40; function 2 is empty; comment 3 is top level.
  fluir::pt::ParseTree makeTree() {
    fluir::pt::FunctionDecl fn;
    fn.id = 1;
    fn.location = kFnLoc;
    fn.body.nodes.emplace(10,
                          fluir::pt::Constant{.id = 10, .location = kNodeLoc, .value = fluir::literals_types::I32{0}});
    fn.body.conduits.emplace(40, fluir::pt::Conduit{.id = 40, .input = 10});

    fluir::pt::FunctionDecl empty;
    empty.id = 2;

    fluir::pt::ParseTree tree;
    tree.declarations.emplace(1, fluir::pt::Declaration{fn});
    tree.declarations.emplace(2, fluir::pt::Declaration{empty});
    tree.declarations.emplace(3, fluir::pt::Declaration{fluir::pt::Comment{.id = 3, .location = kCommentLoc}});
    return tree;
  }

  // Param 2 (I32) and return 4 (F64).
  fluir::pt::FunctionDecl makeRailedFunction() {
    fluir::pt::FunctionDecl fn;
    fn.id = 1;
    fn.input =
      fluir::pt::FunctionDecl::InputBlock{.parameters = {{.id = 2, .index = 0, .name = "a", .typeName = "I32"}}};
    fn.output =
      fluir::pt::FunctionDecl::OutputBlock{.ret = fluir::pt::FunctionDecl::Return{.id = 4, .typeName = "F64"}};
    return fn;
  }

}  // namespace

TEST(TreePath, FunctionAtResolvesAOneSegmentPath) {
  fluir::pt::ParseTree tree = makeTree();

  const fluir::pt::FunctionDecl* fn = functionAt(tree, FullID{1});

  ASSERT_NE(fn, nullptr);
  EXPECT_EQ(fn->id, 1u);
}

TEST(TreePath, FunctionAtMissesUnknownIdsAndOtherDepths) {
  fluir::pt::ParseTree tree = makeTree();

  EXPECT_EQ(functionAt(tree, FullID{999}), nullptr);
  EXPECT_EQ(functionAt(tree, FullID{}), nullptr);
  EXPECT_EQ(functionAt(tree, FullID{1, 10}), nullptr);
}

TEST(TreePath, DeclarationAtResolvesFunctionsAndComments) {
  fluir::pt::ParseTree tree = makeTree();

  const fluir::pt::Declaration* fn = declarationAt(tree, FullID{1});
  const fluir::pt::Declaration* comment = declarationAt(std::as_const(tree), FullID{3});

  ASSERT_NE(fn, nullptr);
  ASSERT_NE(comment, nullptr);
  EXPECT_TRUE(std::holds_alternative<fluir::pt::FunctionDecl>(*fn));
  EXPECT_TRUE(std::holds_alternative<fluir::pt::Comment>(*comment));
}

TEST(TreePath, DeclarationAtMissesUnknownIdsAndOtherDepths) {
  fluir::pt::ParseTree tree = makeTree();

  EXPECT_EQ(declarationAt(tree, FullID{999}), nullptr);
  EXPECT_EQ(declarationAt(tree, FullID{}), nullptr);
  EXPECT_EQ(declarationAt(tree, FullID{1, 10}), nullptr);
}

TEST(TreePath, FunctionAtMissesATopLevelComment) {
  fluir::pt::ParseTree tree = makeTree();

  EXPECT_EQ(functionAt(tree, FullID{3}), nullptr);
}

TEST(TreePath, BlockOfAFunctionIsItsBody) {
  fluir::pt::ParseTree tree = makeTree();

  const fluir::pt::Block* block = blockOf(tree, FullID{1});

  ASSERT_NE(block, nullptr);
  EXPECT_TRUE(block->nodes.contains(10));
}

TEST(TreePath, BlockOfAPlainNodeOrMissIsNull) {
  fluir::pt::ParseTree tree = makeTree();

  EXPECT_EQ(blockOf(tree, FullID{1, 10}), nullptr);
  EXPECT_EQ(blockOf(tree, FullID{999}), nullptr);
  EXPECT_EQ(blockOf(tree, FullID{}), nullptr);
}

TEST(TreePath, NodeAtResolvesANodeInsideItsParentsBlock) {
  fluir::pt::ParseTree tree = makeTree();

  const fluir::pt::Node* node = nodeAt(tree, FullID{1, 10});

  ASSERT_NE(node, nullptr);
  EXPECT_TRUE(std::holds_alternative<fluir::pt::Constant>(*node));
}

TEST(TreePath, NodeAtMissesUnknownNodesConduitsAndShortPaths) {
  fluir::pt::ParseTree tree = makeTree();

  EXPECT_EQ(nodeAt(tree, FullID{1, 999}), nullptr);
  EXPECT_EQ(nodeAt(tree, FullID{2, 10}), nullptr);  // ids are body-scoped
  EXPECT_EQ(nodeAt(tree, FullID{1, 40}), nullptr);  // a conduit is not a node
  EXPECT_EQ(nodeAt(tree, FullID{1}), nullptr);
  EXPECT_EQ(nodeAt(tree, FullID{1, 10, 11}), nullptr);  // a constant has no block
}

TEST(TreePath, LocationAtResolvesFunctionsAndNodes) {
  fluir::pt::ParseTree tree = makeTree();

  const FlowGraphLocation* fnLoc = locationAt(tree, FullID{1});
  const FlowGraphLocation* nodeLoc = locationAt(tree, FullID{1, 10});

  ASSERT_NE(fnLoc, nullptr);
  ASSERT_NE(nodeLoc, nullptr);
  EXPECT_EQ(*fnLoc, kFnLoc);
  EXPECT_EQ(*nodeLoc, kNodeLoc);
}

TEST(TreePath, LocationAtResolvesAndWritesATopLevelComment) {
  fluir::pt::ParseTree tree = makeTree();

  ASSERT_NE(locationAt(tree, FullID{3}), nullptr);
  EXPECT_EQ(*locationAt(tree, FullID{3}), kCommentLoc);
  locationAt(tree, FullID{3})->x = 42;

  EXPECT_EQ(locationAt(std::as_const(tree), FullID{3})->x, 42);
}

TEST(TreePath, LocationAtIsWritable) {
  fluir::pt::ParseTree tree = makeTree();

  locationAt(tree, FullID{1, 10})->x = 42;

  EXPECT_EQ(locationAt(std::as_const(tree), FullID{1, 10})->x, 42);
}

TEST(TreePath, LocationAtMissIsNull) {
  fluir::pt::ParseTree tree = makeTree();

  EXPECT_EQ(locationAt(tree, FullID{}), nullptr);
  EXPECT_EQ(locationAt(tree, FullID{1, 40}), nullptr);
}

TEST(TreePath, ParentOfDropsTheLastSegment) {
  EXPECT_EQ(parentOf(FullID{1, 10}), FullID{1});
  EXPECT_EQ(parentOf(FullID{1}), FullID{});
  EXPECT_EQ(parentOf(FullID{}), FullID{});
}

TEST(TreePath, RailTypeAtFindsAParamType) {
  fluir::pt::FunctionDecl fn = makeRailedFunction();

  const std::string* type = railTypeAt(fn, 2);

  ASSERT_NE(type, nullptr);
  EXPECT_EQ(*type, "I32");
}

TEST(TreePath, RailTypeAtFindsAReturnType) {
  fluir::pt::FunctionDecl fn = makeRailedFunction();

  const std::string* type = railTypeAt(fn, 4);

  ASSERT_NE(type, nullptr);
  EXPECT_EQ(*type, "F64");
}

TEST(TreePath, RailTypeAtMissesUnknownIds) {
  EXPECT_EQ(railTypeAt(makeRailedFunction(), 99), nullptr);
  EXPECT_EQ(railTypeAt(fluir::pt::FunctionDecl{}, 2), nullptr);
}

namespace {

  constexpr FlowGraphLocation kCondLoc{.x = 1, .y = 1, .z = 0, .width = 20, .height = 12};
  constexpr FlowGraphLocation kThenLoc{.x = 0, .y = 0, .z = 0, .width = 20, .height = 5};
  constexpr FlowGraphLocation kElseLoc{.x = 0, .y = 5, .z = 0, .width = 20, .height = 6};
  constexpr FlowGraphLocation kThenNodeLoc{.x = 2, .y = 2, .z = 0, .width = 4, .height = 1};
  constexpr FlowGraphLocation kElseNodeLoc{.x = 3, .y = 3, .z = 0, .width = 4, .height = 1};

  fluir::pt::Constant makeConstant(fluir::ID id, FlowGraphLocation location) {
    return fluir::pt::Constant{.id = id, .location = location, .value = fluir::literals_types::I32{0}};
  }

  // Function 1 holds constant 10 and conditional 20. Both of 20's scopes hold a
  // node with id 1 — the collision a scope-aware path must keep apart.
  fluir::pt::ParseTree makeNestedTree() {
    fluir::pt::Scope thenScope{.id = 0, .location = kThenLoc, .body = {}};
    thenScope.body.nodes.emplace(1, makeConstant(1, kThenNodeLoc));
    thenScope.body.conduits.emplace(50, fluir::pt::Conduit{.id = 50, .input = 1});

    fluir::pt::Scope elseScope{.id = 1, .location = kElseLoc, .body = {}};
    elseScope.body.nodes.emplace(1, makeConstant(1, kElseNodeLoc));

    fluir::pt::Conditional conditional{.id = 20,
                                       .location = kCondLoc,
                                       .condition = {},
                                       .inputs = {},
                                       .outputs = {},
                                       .thenScope = xyz::indirect{std::move(thenScope)},
                                       .elseScope = xyz::indirect{std::move(elseScope)}};

    fluir::pt::ParseTree tree = makeTree();
    std::get<fluir::pt::FunctionDecl>(tree.declarations.at(1)).body.nodes.emplace(20, conditional);
    return tree;
  }

}  // namespace

TEST(TreePath, BlockOfAScopePathIsThatScopesBody) {
  fluir::pt::ParseTree tree = makeNestedTree();

  const fluir::pt::Block* thenBlock = blockOf(tree, FullID{1, 20, 0});
  const fluir::pt::Block* elseBlock = blockOf(tree, FullID{1, 20, 1});

  ASSERT_NE(thenBlock, nullptr);
  ASSERT_NE(elseBlock, nullptr);
  EXPECT_NE(thenBlock, elseBlock);
  EXPECT_TRUE(thenBlock->conduits.contains(50));
  EXPECT_FALSE(elseBlock->conduits.contains(50));
}

TEST(TreePath, NodeAtResolvesANestedNode) {
  fluir::pt::ParseTree tree = makeNestedTree();

  const fluir::pt::Node* node = nodeAt(tree, FullID{1, 20, 0, 1});

  ASSERT_NE(node, nullptr);
  EXPECT_EQ(std::get<fluir::pt::Constant>(*node).location, kThenNodeLoc);
}

TEST(TreePath, TheTwoScopesIdOneNodesAreDifferentNodes) {
  fluir::pt::ParseTree tree = makeNestedTree();

  const FlowGraphLocation* thenNode = locationAt(tree, FullID{1, 20, 0, 1});
  const FlowGraphLocation* elseNode = locationAt(tree, FullID{1, 20, 1, 1});

  ASSERT_NE(thenNode, nullptr);
  ASSERT_NE(elseNode, nullptr);
  EXPECT_EQ(*thenNode, kThenNodeLoc);
  EXPECT_EQ(*elseNode, kElseNodeLoc);
}

TEST(TreePath, BlockOfMissesWrongParityAndUnknownScopes) {
  fluir::pt::ParseTree tree = makeNestedTree();

  EXPECT_EQ(blockOf(tree, FullID{1, 20}), nullptr);        // even depth names a node, not a block
  EXPECT_EQ(blockOf(tree, FullID{1, 20, 7}), nullptr);     // neither then nor else
  EXPECT_EQ(blockOf(tree, FullID{1, 10, 0}), nullptr);     // a constant owns no scope
  EXPECT_EQ(blockOf(tree, FullID{1, 99, 0}), nullptr);     // no such node
  EXPECT_EQ(blockOf(tree, FullID{9, 20, 0}), nullptr);     // no such function
  EXPECT_EQ(blockOf(tree, FullID{1, 20, 0, 1}), nullptr);  // a nested node owns no block
}

TEST(TreePath, NodeAtMissesInsideAnUnresolvableScope) {
  fluir::pt::ParseTree tree = makeNestedTree();

  EXPECT_EQ(nodeAt(tree, FullID{1, 20, 7, 1}), nullptr);
  EXPECT_EQ(nodeAt(tree, FullID{1, 20, 0, 99}), nullptr);
  EXPECT_EQ(nodeAt(tree, FullID{1, 20, 0, 50}), nullptr);  // a conduit is not a node
}

TEST(TreePath, ScopeAtResolvesEitherBranch) {
  fluir::pt::ParseTree tree = makeNestedTree();

  const fluir::pt::Scope* thenScope = scopeAt(tree, FullID{1, 20, 0});
  const fluir::pt::Scope* elseScope = scopeAt(tree, FullID{1, 20, 1});

  ASSERT_NE(thenScope, nullptr);
  ASSERT_NE(elseScope, nullptr);
  EXPECT_EQ(thenScope->location, kThenLoc);
  EXPECT_EQ(elseScope->location, kElseLoc);
}

TEST(TreePath, ScopeAtMissesNonScopePaths) {
  fluir::pt::ParseTree tree = makeNestedTree();

  EXPECT_EQ(scopeAt(tree, FullID{}), nullptr);
  EXPECT_EQ(scopeAt(tree, FullID{1}), nullptr);
  EXPECT_EQ(scopeAt(tree, FullID{1, 20}), nullptr);
  EXPECT_EQ(scopeAt(tree, FullID{1, 20, 7}), nullptr);
  EXPECT_EQ(scopeAt(tree, FullID{1, 10, 0}), nullptr);
}

TEST(TreePath, LocationAtResolvesAndWritesAScopeBand) {
  fluir::pt::ParseTree tree = makeNestedTree();

  FlowGraphLocation* band = locationAt(tree, FullID{1, 20, 1});

  ASSERT_NE(band, nullptr);
  EXPECT_EQ(*band, kElseLoc);
  band->height = 9;

  EXPECT_EQ(locationAt(std::as_const(tree), FullID{1, 20, 1})->height, 9);
}

TEST(TreePath, PathKindsFollowDepthParity) {
  EXPECT_FALSE(isNodePath(FullID{}));
  EXPECT_FALSE(isNodePath(FullID{1}));
  EXPECT_TRUE(isNodePath(FullID{1, 20}));
  EXPECT_FALSE(isNodePath(FullID{1, 20, 0}));
  EXPECT_TRUE(isNodePath(FullID{1, 20, 0, 1}));

  EXPECT_FALSE(isScopePath(FullID{}));
  EXPECT_FALSE(isScopePath(FullID{1}));
  EXPECT_FALSE(isScopePath(FullID{1, 20}));
  EXPECT_TRUE(isScopePath(FullID{1, 20, 0}));
  EXPECT_FALSE(isScopePath(FullID{1, 20, 0, 1}));
  EXPECT_TRUE(isScopePath(FullID{1, 20, 0, 1, 0}));
}
