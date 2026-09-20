#include "editor/core/intelligence.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/transaction/add_comment.hpp"
#include "tool_harness.hpp"

// simple_binary_expr.fl: function 1, binary 1, constant 2. simple_unary_expr.fl: function 1, unary 7, constant 3.
// function_with_input_only.fl: function 1, params 2 and 3. function_with_output_only.fl: function 1, return 4.

namespace {

  using fluir::FullID;
  using fluir::Operator;
  using fluir::editor::CommentOption;
  using fluir::editor::Completion;
  using fluir::editor::ConstantOption;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::Intelligence;
  using fluir::editor::OperatorOption;
  using Literal = fluir::literals_types::Literal;

  const EditorContext ctx;
  const std::vector<std::string_view> BUILTIN_TYPES{
    "F64", "I8", "I16", "I32", "I64", "U8", "U16", "U32", "U64", "BOOL"};

}  // namespace

TEST(Intelligence, ABinaryNodeOffersTheBinaryOperators) {
  EditorState state{ctx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");

  const std::vector<Operator> expected{Operator::PLUS,
                                       Operator::MINUS,
                                       Operator::STAR,
                                       Operator::SLASH,
                                       Operator::EQUAL_EQUAL,
                                       Operator::BANG_EQUAL,
                                       Operator::GREATER_EQUAL,
                                       Operator::LESS_EQUAL,
                                       Operator::GREATER,
                                       Operator::LESS,
                                       Operator::AND_AND,
                                       Operator::BAR_BAR};
  EXPECT_EQ(Intelligence{}.operators(state.editor.tree(), FullID{1, 1}), expected);
}

TEST(Intelligence, AUnaryNodeOffersTheUnaryOperators) {
  EditorState state{ctx};
  testutil::loadInto(state, "read/simple_unary_expr.fl");

  const std::vector<Operator> expected{
    Operator::PLUS, Operator::MINUS, Operator::PLUS_PLUS, Operator::MINUS_MINUS, Operator::BANG};
  EXPECT_EQ(Intelligence{}.operators(state.editor.tree(), FullID{1, 7}), expected);
}

TEST(Intelligence, NonOperatorsAndMissingPathsOfferNothing) {
  EditorState state{ctx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");
  const Intelligence uut;

  EXPECT_TRUE(uut.operators(state.editor.tree(), FullID{1, 2}).empty()) << "constant";
  EXPECT_TRUE(uut.operators(state.editor.tree(), FullID{1}).empty()) << "function";
  EXPECT_TRUE(uut.operators(state.editor.tree(), FullID{1, 99}).empty()) << "missing";
  EXPECT_TRUE(uut.operators(state.editor.tree(), FullID{}).empty()) << "empty path";
}

TEST(Intelligence, ACallOffersNothing) {
  EditorState state{ctx};
  testutil::loadInto(state, "read/function_call.fl");
  const Intelligence uut;

  EXPECT_TRUE(uut.operators(state.editor.tree(), FullID{1, 3}).empty());
}

TEST(Intelligence, AParamRailOffersTheBuiltinTypes) {
  EditorState state{ctx};
  testutil::loadInto(state, "read/function_with_input_only.fl");

  EXPECT_EQ(Intelligence{}.types(state.editor.tree(), FullID{1, 2}), BUILTIN_TYPES);
}

TEST(Intelligence, AReturnRailOffersTheBuiltinTypes) {
  EditorState state{ctx};
  testutil::loadInto(state, "read/function_with_output_only.fl");

  EXPECT_EQ(Intelligence{}.types(state.editor.tree(), FullID{1, 4}), BUILTIN_TYPES);
}

TEST(Intelligence, NonRailsAndMissingPathsOfferNoTypes) {
  EditorState state{ctx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");
  const Intelligence uut;

  EXPECT_TRUE(uut.types(state.editor.tree(), FullID{1}).empty()) << "function";
  EXPECT_TRUE(uut.types(state.editor.tree(), FullID{1, 1}).empty()) << "node";
  EXPECT_TRUE(uut.types(state.editor.tree(), FullID{1, 99}).empty()) << "missing";
  EXPECT_TRUE(uut.types(state.editor.tree(), FullID{}).empty()) << "empty path";
}

TEST(Intelligence, TheTopLevelOffersAFunctionThenAComment) {
  EditorState state{ctx};
  testutil::loadInto(state, "read/single_empty_function.fl");

  const std::vector<fluir::editor::Completion> got = Intelligence{}.completions(state.editor.tree(), FullID{});

  ASSERT_EQ(got.size(), 2u);
  EXPECT_EQ(got[0].label, "Function");
  EXPECT_TRUE(std::holds_alternative<fluir::editor::FunctionDefOption>(got[0].option));
  EXPECT_EQ(got[1].label, "Comment");
  EXPECT_TRUE(std::holds_alternative<fluir::editor::CommentOption>(got[1].option));
}

TEST(Intelligence, AFunctionBodyOffersBinaryThenUnaryOperatorsThenConstantsThenAComment) {
  EditorState state{ctx};
  testutil::loadInto(state, "read/single_empty_function.fl");
  const std::vector<Operator> binary{Operator::PLUS,
                                     Operator::MINUS,
                                     Operator::STAR,
                                     Operator::SLASH,
                                     Operator::EQUAL_EQUAL,
                                     Operator::BANG_EQUAL,
                                     Operator::GREATER_EQUAL,
                                     Operator::LESS_EQUAL,
                                     Operator::GREATER,
                                     Operator::LESS,
                                     Operator::AND_AND,
                                     Operator::BAR_BAR};
  const std::vector<Operator> unary{
    Operator::PLUS, Operator::MINUS, Operator::PLUS_PLUS, Operator::MINUS_MINUS, Operator::BANG};

  const std::vector<Completion> actual = Intelligence{}.completions(state.editor.tree(), FullID{1});

  for (Operator op : binary) {
    auto expected = std::format("{} (binary)", fluir::stringify(op));
    auto found = std::ranges::find_if(actual, [&](const auto& o) { return o.label == expected; });
    EXPECT_NE(actual.end(), found) << "MISSING " << expected;
  }
  for (Operator op : unary) {
    auto expected = std::format("{} (unary)", fluir::stringify(op));
    auto found = std::ranges::find_if(actual, [&](const auto& o) { return o.label == expected; });
    EXPECT_NE(actual.end(), found) << "MISSING " << expected;
  }
  for (const auto& expected : BUILTIN_TYPES) {
    auto found = std::ranges::find_if(actual, [&](const auto& o) { return o.label == expected; });
    EXPECT_NE(actual.end(), found) << "MISSING " << expected;
  }

  {
    std::string expected = "Comment";
    auto found = std::ranges::find_if(actual, [&](const auto& o) { return o.label == expected; });
    EXPECT_NE(actual.end(), found) << "MISSING " << expected;
  }
}

TEST(Intelligence, ConstantCompletionsDefaultToZeroOrFalse) {
  EditorState state{ctx};
  testutil::loadInto(state, "read/single_empty_function.fl");

  const std::vector<Completion> got = Intelligence{}.completions(state.editor.tree(), FullID{1});
  const auto valueOf = [&](std::string_view label) {
    const auto it = std::ranges::find(got, label, &Completion::label);
    const auto* option = it == got.end() ? nullptr : std::get_if<ConstantOption>(&it->option);
    if (option == nullptr) {
      ADD_FAILURE() << "no constant " << label;
      return Literal{};
    }
    return option->value;
  };

  EXPECT_EQ(valueOf("F64"), (Literal{fluir::literals_types::F64{0.0}}));
  EXPECT_EQ(valueOf("I32"), (Literal{fluir::literals_types::I32{0}}));
  EXPECT_EQ(valueOf("U8"), (Literal{fluir::literals_types::U8{0}}));
  EXPECT_EQ(valueOf("BOOL"), (Literal{false}));
}

TEST(Intelligence, UnknownBodiesAndCommentsOfferNoCompletions) {
  EditorState state{ctx};
  testutil::loadInto(state, "read/single_empty_function.fl");
  state.editor.apply(std::make_unique<fluir::editor::AddComment>(
    FullID{}, 50, fluir::FlowGraphLocation{.x = 0, .y = 0, .z = 1, .width = 10, .height = 10}));
  const Intelligence uut;

  EXPECT_TRUE(uut.completions(state.editor.tree(), FullID{99}).empty()) << "missing";
  EXPECT_TRUE(uut.completions(state.editor.tree(), FullID{50}).empty()) << "comment";
}

// A branch is a block, so it offers the same completions a function body does.
TEST(Intelligence, CompletionsReachIntoABranch) {
  fluir::pt::Scope then{.id = 0, .location = {.x = 0, .y = 0, .z = 0, .width = 20, .height = 12}, .body = {}};
  fluir::pt::FunctionDecl fn;
  fn.id = 1;
  fn.name = "f";
  fn.body.nodes.emplace(
    20,
    fluir::pt::Conditional{.id = 20,
                           .location = {.x = 0, .y = 0, .z = 0, .width = 20, .height = 18},
                           .condition = {},
                           .inputs = {},
                           .outputs = {},
                           .thenScope = xyz::indirect{std::move(then)},
                           .elseScope = xyz::indirect{fluir::pt::Scope{
                             .id = 1, .location = {.x = 0, .y = 12, .z = 0, .width = 20, .height = 6}, .body = {}}}});
  fluir::pt::ParseTree tree;
  tree.declarations.emplace(1, fluir::pt::Declaration{std::move(fn)});

  const Intelligence uut;

  const std::vector<Completion> branch = uut.completions(tree, FullID{1, 20, 0});
  const std::vector<Completion> body = uut.completions(tree, FullID{1});
  ASSERT_EQ(branch.size(), body.size());
  for (std::size_t i = 0; i < branch.size(); ++i) {
    EXPECT_EQ(branch[i].label, body[i].label);
  }
  EXPECT_TRUE(uut.completions(tree, FullID{1, 20, 7}).empty()) << "no such branch";
}
