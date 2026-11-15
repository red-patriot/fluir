#include "editor/core/scope_path.hpp"

#include <utility>
#include <variant>

#include <gtest/gtest.h>

// A scope path names a function body, then a conditional and branch per level down.

namespace {

  namespace pt = fluir::pt;
  using fluir::editor::blockAt;
  using fluir::editor::ELSE_BRANCH_ID;
  using fluir::editor::THEN_BRANCH_ID;

  pt::Conditional conditional(fluir::ID id) {
    return {.id = id,
            .location = {},
            .condition = {.innerId = 1, .y = 10},
            .inputs = {},
            .outputs = {},
            .thenScope = xyz::indirect<pt::Block>{},
            .elseScope = xyz::indirect<pt::Block>{}};
  }

  // Function 1 holds conditional 2, whose then branch holds conditional 3.
  pt::ParseTree nested() {
    pt::Conditional outer = conditional(2);
    outer.thenScope->nodes.emplace(3, conditional(3));
    pt::FunctionDecl main{.id = 1, .location = {}, .name = "main", .body = {}, .input = {}, .output = {}};
    main.body.nodes.emplace(2, std::move(outer));
    pt::ParseTree tree;
    tree.declarations.emplace(1, std::move(main));
    tree.declarations.emplace(4, pt::Comment{.id = 4, .location = {}, .text = ""});
    return tree;
  }

  const pt::Conditional& conditionalIn(const pt::Block& block, fluir::ID id) {
    return std::get<pt::Conditional>(block.nodes.at(id));
  }

}  // namespace

TEST(ScopePath, AFunctionIdNamesItsBody) {
  const pt::ParseTree tree = nested();

  EXPECT_EQ(blockAt(tree, {1}), &std::get<pt::FunctionDecl>(tree.declarations.at(1)).body);
}

TEST(ScopePath, AConditionalAndBranchNameThatBranch) {
  const pt::ParseTree tree = nested();
  const pt::Block& body = std::get<pt::FunctionDecl>(tree.declarations.at(1)).body;
  const pt::Conditional& outer = conditionalIn(body, 2);
  const pt::Conditional& inner = conditionalIn(*outer.thenScope, 3);

  EXPECT_EQ(blockAt(tree, {1, 2, THEN_BRANCH_ID}), &*outer.thenScope);
  EXPECT_EQ(blockAt(tree, {1, 2, ELSE_BRANCH_ID}), &*outer.elseScope);
  EXPECT_EQ(blockAt(tree, {1, 2, THEN_BRANCH_ID, 3, ELSE_BRANCH_ID}), &*inner.elseScope);
}

TEST(ScopePath, APathThatNamesNoBodyNamesNothing) {
  const pt::ParseTree tree = nested();

  EXPECT_EQ(blockAt(tree, {}), nullptr);
  EXPECT_EQ(blockAt(tree, {99}), nullptr);
  EXPECT_EQ(blockAt(tree, {4}), nullptr);                                        // a comment has no body
  EXPECT_EQ(blockAt(tree, {1, 2}), nullptr);                                     // a node, not a body
  EXPECT_EQ(blockAt(tree, {1, 2, 3}), nullptr);                                  // no such branch
  EXPECT_EQ(blockAt(tree, {1, 9, THEN_BRANCH_ID}), nullptr);                     // no such conditional
  EXPECT_EQ(blockAt(tree, {1, 2, ELSE_BRANCH_ID, 3, THEN_BRANCH_ID}), nullptr);  // 3 is in the other branch
}
