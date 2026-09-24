#include "editor/tools/choice_tool.hpp"

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/tools/menu_popup.hpp"
#include "tool_harness.hpp"

// simple_binary_expr.fl: binary 1 {125,85,25,25}, constant 2 {60,85,25,25}. simple_unary_expr.fl: unary 7 at the
// same rect. function_with_input_only.fl: function 1 at {50,50}; param a (I32) rail {50,75,75,25}.
// function_with_output_only.fl: return 4 (F64) rail {525,75,25,25}. Presses land clear of move grips.

namespace {

  using fluir::editor::ChoiceTool;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::InputEvent;
  using fluir::editor::MenuPopup;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::send;

  const EditorContext kCtx;
  constexpr Vec2 kOperatorBody{127, 107};
  constexpr Vec2 kConstantBody{62, 103};
  constexpr Vec2 kParamATag{55, 90};
  constexpr Vec2 kParamAName{100, 90};
  constexpr Vec2 kHeader{100, 55};
  constexpr Vec2 kReturnTag{527, 80};
  const std::vector<std::string> kBuiltins{"F64", "I8", "I16", "I32", "I64", "U8", "U16", "U32", "U64", "BOOL"};

  std::vector<std::string> menuLabels(const EditorState& state) {
    const auto* menu = dynamic_cast<const MenuPopup*>(state.popup.get());
    return menu == nullptr ? std::vector<std::string>{} : menu->labels();
  }

}  // namespace

TEST(ChoiceTool, PressingABinaryNodeOpensItsOperatorsWithoutConsuming) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");
  ChoiceTool uut;

  EXPECT_FALSE(send(uut, state, down(kOperatorBody)));

  const std::vector<std::string> want{"+", "-", "*", "/", "==", "!=", ">=", "<=", ">", "<", "&&", "||"};
  EXPECT_EQ(menuLabels(state), want);
}

TEST(ChoiceTool, PressingAUnaryNodeOpensItsOperators) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_unary_expr.fl");
  ChoiceTool uut;

  send(uut, state, down(kOperatorBody));

  const std::vector<std::string> want{"+", "-", "++", "--", "!"};
  EXPECT_EQ(menuLabels(state), want);
}

TEST(ChoiceTool, PressingAParamTagOpensTheBuiltinTypesWithoutConsuming) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/function_with_input_only.fl");
  ChoiceTool uut;

  EXPECT_FALSE(send(uut, state, down(kParamATag)));

  EXPECT_EQ(menuLabels(state), kBuiltins);
}

TEST(ChoiceTool, PressingTheReturnTagOpensTheBuiltinTypes) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/function_with_output_only.fl");
  ChoiceTool uut;

  send(uut, state, down(kReturnTag));

  EXPECT_EQ(menuLabels(state), kBuiltins);
}

TEST(ChoiceTool, PressingATextFieldTheHeaderOrTheBackgroundOpensNothing) {
  EditorState operators{kCtx};
  testutil::loadInto(operators, "read/simple_binary_expr.fl");
  EditorState rails{kCtx};
  testutil::loadInto(rails, "read/function_with_input_only.fl");
  ChoiceTool uut;

  EXPECT_FALSE(send(uut, operators, down(kConstantBody)));
  EXPECT_FALSE(send(uut, rails, down(kParamAName)));
  EXPECT_FALSE(send(uut, rails, down(kHeader)));
  EXPECT_FALSE(send(uut, rails, down(Vec2{-10, -10})));

  EXPECT_EQ(operators.popup, nullptr);
  EXPECT_EQ(rails.popup, nullptr);
}

TEST(ChoiceTool, OnlyALeftPressOpens) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");
  ChoiceTool uut;

  send(uut, state, down(kOperatorBody, InputEvent::Button::Right));

  EXPECT_EQ(state.popup, nullptr);
}

TEST(ChoiceTool, PressesHonourTheViewport) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/function_with_input_only.fl");
  state.view.pan = Vec2{100, 0};
  ChoiceTool uut;

  send(uut, state, down(kParamATag + Vec2{100, 0}));

  EXPECT_NE(state.popup, nullptr);
}

TEST(ChoiceTool, TheTagRegionIsWorldFixedUnderZoom) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/function_with_input_only.fl");
  state.view.scale = 2.0;
  ChoiceTool uut;

  // World x 68 sits inside the tag's world-px width (textPad + 3 glyphs at 0.8x).
  send(uut, state, down(Vec2{68, 90} * 2.0));

  EXPECT_EQ(menuLabels(state), kBuiltins);
}
