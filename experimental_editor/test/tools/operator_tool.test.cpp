#include "editor/tools/operator_tool.hpp"

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/tools/menu_popup.hpp"
#include "tool_harness.hpp"

// simple_binary_expr.fl: binary 1 {125,85,25,25}, constant 2 {60,85,25,25}. simple_unary_expr.fl: unary 7 at the
// same rect. Presses land on bodies clear of the move grip.

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::InputEvent;
  using fluir::editor::MenuPopup;
  using fluir::editor::OperatorTool;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::send;

  const EditorContext kCtx;
  constexpr Vec2 kOperatorBody{127, 107};
  constexpr Vec2 kConstantBody{62, 103};

  std::vector<std::string> menuLabels(const EditorState& state) {
    const auto* menu = dynamic_cast<const MenuPopup*>(state.popup.get());
    return menu == nullptr ? std::vector<std::string>{} : menu->labels();
  }

}  // namespace

TEST(OperatorTool, PressingABinaryNodeOpensItsOperatorsWithoutConsuming) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");
  OperatorTool uut;

  EXPECT_FALSE(send(uut, state, down(kOperatorBody)));

  const std::vector<std::string> want{"+", "-", "*", "/", "==", "!=", ">=", "<=", ">", "<", "&&", "||"};
  EXPECT_EQ(menuLabels(state), want);
}

TEST(OperatorTool, PressingAUnaryNodeOpensItsOperators) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_unary_expr.fl");
  OperatorTool uut;

  send(uut, state, down(kOperatorBody));

  const std::vector<std::string> want{"+", "-", "++", "--", "!"};
  EXPECT_EQ(menuLabels(state), want);
}

TEST(OperatorTool, PressingAnythingElseOpensNothing) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");
  OperatorTool uut;

  EXPECT_FALSE(send(uut, state, down(kConstantBody)));
  EXPECT_FALSE(send(uut, state, down(Vec2{-10, -10})));

  EXPECT_EQ(state.popup, nullptr);
}

TEST(OperatorTool, OnlyALeftPressOpens) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");
  OperatorTool uut;

  send(uut, state, down(kOperatorBody, InputEvent::Button::Right));

  EXPECT_EQ(state.popup, nullptr);
}

TEST(OperatorTool, PressesHonourTheViewport) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");
  state.view.pan = Vec2{100, 0};
  OperatorTool uut;

  send(uut, state, down(kOperatorBody + Vec2{100, 0}));

  EXPECT_NE(state.popup, nullptr);
}
