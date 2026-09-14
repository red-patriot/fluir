#include "editor/core/intelligence.hpp"

#include <vector>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "compiler/models/operator.hpp"
#include "editor/core/editor_context.hpp"
#include "tool_harness.hpp"

// simple_binary_expr.fl: function 1, binary 1, constant 2. simple_unary_expr.fl: function 1, unary 7, constant 3.

namespace {

  using fluir::FullID;
  using fluir::Operator;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::Intelligence;

  const EditorContext kCtx;

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
