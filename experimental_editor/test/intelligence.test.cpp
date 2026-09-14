#include "editor/core/intelligence.hpp"

#include <string_view>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "compiler/models/operator.hpp"
#include "editor/core/editor_context.hpp"
#include "tool_harness.hpp"

// simple_binary_expr.fl: function 1, binary 1, constant 2. simple_unary_expr.fl: function 1, unary 7, constant 3.
// function_with_input_only.fl: function 1, params 2 and 3. function_with_output_only.fl: function 1, return 4.

namespace {

  using fluir::FullID;
  using fluir::Operator;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::Intelligence;

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

TEST(Intelligence, AFunctionBodyOffersNoCompletionsYet) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/single_empty_function.fl");

  EXPECT_TRUE(Intelligence{}.completions(state.editor.tree(), FullID{1}).empty());
}
