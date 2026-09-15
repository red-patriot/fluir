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

  const EditorContext kCtx;
  const std::vector<std::string_view> kBuiltins{"F64", "I8", "I16", "I32", "I64", "U8", "U16", "U32", "U64", "BOOL"};

}  // namespace

TEST(Intelligence, ABinaryNodeOffersTheBinaryOperators) {
  EditorState state{kCtx};
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
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_unary_expr.fl");

  const std::vector<Operator> expected{
    Operator::PLUS, Operator::MINUS, Operator::PLUS_PLUS, Operator::MINUS_MINUS, Operator::BANG};
  EXPECT_EQ(Intelligence{}.operators(state.editor.tree(), FullID{1, 7}), expected);
}

TEST(Intelligence, NonOperatorsAndMissingPathsOfferNothing) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");
  const Intelligence uut;

  EXPECT_TRUE(uut.operators(state.editor.tree(), FullID{1, 2}).empty()) << "constant";
  EXPECT_TRUE(uut.operators(state.editor.tree(), FullID{1}).empty()) << "function";
  EXPECT_TRUE(uut.operators(state.editor.tree(), FullID{1, 99}).empty()) << "missing";
  EXPECT_TRUE(uut.operators(state.editor.tree(), FullID{}).empty()) << "empty path";
}

TEST(Intelligence, ACallOffersNothing) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/function_call.fl");
  const Intelligence uut;

  EXPECT_TRUE(uut.operators(state.editor.tree(), FullID{1, 3}).empty());
}

TEST(Intelligence, AParamRailOffersTheBuiltinTypes) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/function_with_input_only.fl");

  EXPECT_EQ(Intelligence{}.types(state.editor.tree(), FullID{1, 2}), kBuiltins);
}

TEST(Intelligence, AReturnRailOffersTheBuiltinTypes) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/function_with_output_only.fl");

  EXPECT_EQ(Intelligence{}.types(state.editor.tree(), FullID{1, 4}), kBuiltins);
}

TEST(Intelligence, NonRailsAndMissingPathsOfferNoTypes) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");
  const Intelligence uut;

  EXPECT_TRUE(uut.types(state.editor.tree(), FullID{1}).empty()) << "function";
  EXPECT_TRUE(uut.types(state.editor.tree(), FullID{1, 1}).empty()) << "node";
  EXPECT_TRUE(uut.types(state.editor.tree(), FullID{1, 99}).empty()) << "missing";
  EXPECT_TRUE(uut.types(state.editor.tree(), FullID{}).empty()) << "empty path";
}

TEST(Intelligence, TheTopLevelOffersAFunctionThenAComment) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/single_empty_function.fl");

  const std::vector<fluir::editor::Completion> got = Intelligence{}.completions(state.editor.tree(), FullID{});

  ASSERT_EQ(got.size(), 2u);
  EXPECT_EQ(got[0].label, "Function");
  EXPECT_TRUE(std::holds_alternative<fluir::editor::FunctionDefOption>(got[0].option));
  EXPECT_EQ(got[1].label, "Comment");
  EXPECT_TRUE(std::holds_alternative<fluir::editor::CommentOption>(got[1].option));
}

TEST(Intelligence, AFunctionBodyOffersBinaryThenUnaryOperatorsThenConstantsThenAComment) {
  EditorState state{kCtx};
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

  const std::vector<Completion> got = Intelligence{}.completions(state.editor.tree(), FullID{1});

  ASSERT_EQ(got.size(), binary.size() + unary.size() + kBuiltins.size() + 1);
  std::size_t at = 0;
  for (Operator op : binary) {
    EXPECT_EQ(got[at].label, std::string{fluir::stringify(op)} + " (binary)");
    const auto* option = std::get_if<OperatorOption>(&got[at].option);
    ASSERT_NE(option, nullptr) << got[at].label;
    EXPECT_EQ(option->op, op);
    EXPECT_EQ(option->arity, OperatorOption::BINARY);
    ++at;
  }
  for (Operator op : unary) {
    EXPECT_EQ(got[at].label, std::string{fluir::stringify(op)} + " (unary)");
    const auto* option = std::get_if<OperatorOption>(&got[at].option);
    ASSERT_NE(option, nullptr) << got[at].label;
    EXPECT_EQ(option->op, op);
    EXPECT_EQ(option->arity, OperatorOption::UNARY);
    ++at;
  }
  for (std::size_t i = 0; i < kBuiltins.size(); ++i, ++at) {
    EXPECT_EQ(got[at].label, kBuiltins[i]);
    const auto* option = std::get_if<ConstantOption>(&got[at].option);
    ASSERT_NE(option, nullptr) << got[at].label;
    EXPECT_EQ(option->value.index(), i);
  }
  EXPECT_EQ(got[at].label, "Comment");
  EXPECT_TRUE(std::holds_alternative<CommentOption>(got[at].option));
}

TEST(Intelligence, ConstantCompletionsDefaultToZeroOrFalse) {
  EditorState state{kCtx};
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
  EditorState state{kCtx};
  testutil::loadInto(state, "read/single_empty_function.fl");
  state.editor.apply(std::make_unique<fluir::editor::AddComment>(
    FullID{}, 50, fluir::FlowGraphLocation{.x = 0, .y = 0, .z = 1, .width = 10, .height = 10}));
  const Intelligence uut;

  EXPECT_TRUE(uut.completions(state.editor.tree(), FullID{99}).empty()) << "missing";
  EXPECT_TRUE(uut.completions(state.editor.tree(), FullID{50}).empty()) << "comment";
}
