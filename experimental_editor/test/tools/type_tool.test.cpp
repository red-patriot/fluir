#include "editor/tools/type_tool.hpp"

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/tools/menu_popup.hpp"
#include "tool_harness.hpp"

// function_with_input_only.fl: function 1 at {50,50}; param a (id 2, I32) rail {50,75,75,25}.
// function_with_output_only.fl: return 4 (F64) rail {525,75,25,25}.

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::InputEvent;
  using fluir::editor::MenuPopup;
  using fluir::editor::TypeTool;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::send;

  const EditorContext kCtx;
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

TEST(TypeTool, PressingAParamTagOpensTheBuiltinTypesWithoutConsuming) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/function_with_input_only.fl");
  TypeTool uut;

  EXPECT_FALSE(send(uut, state, down(kParamATag)));

  EXPECT_EQ(menuLabels(state), kBuiltins);
}

TEST(TypeTool, PressingTheReturnTagOpensTheMenu) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/function_with_output_only.fl");
  TypeTool uut;

  send(uut, state, down(kReturnTag));

  EXPECT_EQ(menuLabels(state), kBuiltins);
}

TEST(TypeTool, PressingANameTheHeaderOrTheBackgroundOpensNothing) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/function_with_input_only.fl");
  TypeTool uut;

  EXPECT_FALSE(send(uut, state, down(kParamAName)));
  EXPECT_FALSE(send(uut, state, down(kHeader)));
  EXPECT_FALSE(send(uut, state, down(Vec2{-10, -10})));

  EXPECT_EQ(state.popup, nullptr);
}

TEST(TypeTool, OnlyALeftPressOpens) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/function_with_input_only.fl");
  TypeTool uut;

  send(uut, state, down(kParamATag, InputEvent::Button::Right));

  EXPECT_EQ(state.popup, nullptr);
}

TEST(TypeTool, PressesHonourTheViewport) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/function_with_input_only.fl");
  state.view.pan = Vec2{100, 0};
  TypeTool uut;

  send(uut, state, down(kParamATag + Vec2{100, 0}));

  EXPECT_NE(state.popup, nullptr);
}

TEST(TypeTool, TheTagRegionIsWorldFixedUnderZoom) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/function_with_input_only.fl");
  state.view.scale = 2.0;
  TypeTool uut;

  // World x 68 sits inside the tag's world-px width (textPad + 3 glyphs at 0.8x).
  send(uut, state, down(Vec2{68, 90} * 2.0));

  EXPECT_EQ(menuLabels(state), kBuiltins);
}
