#include "editor/core/node_access.hpp"

#include <gtest/gtest.h>

#include "editor/core/tree.hpp"

namespace {

  using fluir::FlowGraphLocation;
  using fluir::FullID;
  namespace et = fluir::editor::et;
  namespace editor = fluir::editor;

  constexpr FlowGraphLocation kAt{.x = 1, .y = 2, .z = 3, .width = 4, .height = 5};

  et::ParseTree treeWithComments() {
    et::FunctionDecl fn{.id = 1, .location = kAt, .name = "f", .body = {}, .input = {}, .output = {}};
    fn.body.nodes.emplace(2, et::Comment{.id = 2, .location = kAt, .text = "inner"});
    fn.body.nodes.emplace(3, et::Constant{.id = 3, .location = kAt, .value = fluir::literals_types::I32{7}});
    et::ParseTree tree;
    tree.declarations.emplace(1, std::move(fn));
    tree.declarations.emplace(4, et::Comment{.id = 4, .location = kAt, .text = "outer"});
    return tree;
  }

  TEST(NodeAccess, IdAndLocationOfNode) {
    const et::Node node = et::Constant{.id = 9, .location = kAt, .value = fluir::literals_types::I32{0}};
    EXPECT_EQ(editor::idOf(node), 9u);
    EXPECT_EQ(editor::locationOf(node), kAt);
  }

  TEST(NodeAccess, IdAndLocationOfDeclaration) {
    const et::Declaration decl = et::Comment{.id = 6, .location = kAt, .text = ""};
    EXPECT_EQ(editor::idOf(decl), 6u);
    EXPECT_EQ(editor::locationOf(decl), kAt);
  }

  TEST(NodeAccess, SortedArgumentsOrdersByIndex) {
    const et::Call call{.id = 1, .location = kAt, .target = "g", ._return = {}, .arguments = {{"b", 1}, {"a", 0}}};
    const auto args = editor::sortedArguments(call);
    ASSERT_EQ(args.size(), 2u);
    EXPECT_EQ(args[0]->name, "a");
    EXPECT_EQ(args[1]->name, "b");
  }

  TEST(NodeAccess, SortedParametersOrdersByIndexAndIsEmptyWithoutInput) {
    et::FunctionDecl fn{.id = 1, .location = kAt, .name = "f", .body = {}, .input = {}, .output = {}};
    EXPECT_TRUE(editor::sortedParameters(fn).empty());
    fn.input = et::FunctionDecl::InputBlock{
      {{.id = 5, .index = 1, .name = "y", .typeName = "I32"}, {.id = 6, .index = 0, .name = "x", .typeName = "I32"}}};
    const auto params = editor::sortedParameters(fn);
    ASSERT_EQ(params.size(), 2u);
    EXPECT_EQ(params[0]->name, "x");
    EXPECT_EQ(params[1]->name, "y");
  }

  TEST(NodeAccess, CommentAtFindsDeclarationAndNodeComments) {
    et::ParseTree tree = treeWithComments();
    const et::ParseTree& constTree = tree;
    ASSERT_NE(editor::commentAt(constTree, FullID{4}), nullptr);
    EXPECT_EQ(editor::commentAt(constTree, FullID{4})->text, "outer");
    ASSERT_NE(editor::commentAt(constTree, FullID{1, 2}), nullptr);
    EXPECT_EQ(editor::commentAt(constTree, FullID{1, 2})->text, "inner");
    editor::commentAt(tree, FullID{1, 2})->text = "changed";
    EXPECT_EQ(editor::commentAt(constTree, FullID{1, 2})->text, "changed");
  }

  TEST(NodeAccess, CommentAtIsNullForOtherKindsAndMissingPaths) {
    const et::ParseTree tree = treeWithComments();
    EXPECT_EQ(editor::commentAt(tree, FullID{1}), nullptr);
    EXPECT_EQ(editor::commentAt(tree, FullID{1, 3}), nullptr);
    EXPECT_EQ(editor::commentAt(tree, FullID{1, 99}), nullptr);
    EXPECT_EQ(editor::commentAt(tree, FullID{}), nullptr);
  }

}  // namespace
