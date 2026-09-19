#include "editor/tools/bool_toggle_tool.hpp"

#include <variant>

#include <gtest/gtest.h>

#include "compiler/models/literal_types.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/tree_path.hpp"
#include "tool_harness.hpp"

// read_boolean.fl: false node 1 {60,175,40,25} with square {64,181.5,12,12}; true node 2 {110,180,40,25}
// with square {114,186.5,12,12}. Move grips sit at {80,180,15,15} and {130,185,15,15}.
// int_constants.fl: node 1 is an I8, so its square never exists.

namespace {

  using fluir::FullID;
  using fluir::editor::BoolToggleTool;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::InputEvent;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::send;

  const EditorContext kCtx;
  constexpr Vec2 kFalseSquare{70, 187};
  constexpr Vec2 kTrueSquare{120, 192};
  constexpr Vec2 kFalseGrip{85, 187};

  bool valueOf(const EditorState& state, const FullID& path) {
    const fluir::pt::Node* node = fluir::editor::nodeAt(state.editor.tree(), path);
    return std::get<fluir::literals_types::BOOL>(std::get<fluir::pt::Constant>(*node).value);
  }

}  // namespace

TEST(BoolToggleTool, PressingTheSquareTogglesAndConsumes) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/read_boolean.fl");
  BoolToggleTool uut;

  EXPECT_TRUE(send(uut, state, down(kFalseSquare)));
  EXPECT_TRUE(valueOf(state, FullID{1, 1}));

  EXPECT_TRUE(send(uut, state, down(kTrueSquare)));
  EXPECT_FALSE(valueOf(state, FullID{1, 2}));
}

TEST(BoolToggleTool, UndoRestoresTheValue) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/read_boolean.fl");
  BoolToggleTool uut;

  send(uut, state, down(kFalseSquare));
  ASSERT_TRUE(state.editor.canUndo());
  state.editor.undo();

  EXPECT_FALSE(valueOf(state, FullID{1, 1}));
}

TEST(BoolToggleTool, PressingOutsideTheSquareDoesNothing) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/read_boolean.fl");
  BoolToggleTool uut;

  EXPECT_FALSE(send(uut, state, down(kFalseGrip)));      // the move grip's half of the node
  EXPECT_FALSE(send(uut, state, down(Vec2{62, 177})));   // node body, above-left of the square
  EXPECT_FALSE(send(uut, state, down(Vec2{-10, -10})));  // empty canvas

  EXPECT_FALSE(valueOf(state, FullID{1, 1}));
  EXPECT_FALSE(state.editor.canUndo());
}

TEST(BoolToggleTool, OnlyALeftPressToggles) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/read_boolean.fl");
  BoolToggleTool uut;

  EXPECT_FALSE(send(uut, state, down(kFalseSquare, InputEvent::Button::Right)));

  EXPECT_FALSE(valueOf(state, FullID{1, 1}));
}

TEST(BoolToggleTool, NonBoolConstantsAreIgnored) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/int_constants.fl");
  BoolToggleTool uut;

  EXPECT_FALSE(send(uut, state, down(Vec2{70, 187})));

  EXPECT_FALSE(state.editor.canUndo());
}

TEST(BoolToggleTool, PressesHonourTheViewport) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/read_boolean.fl");
  state.view.pan = Vec2{100, 0};
  BoolToggleTool uut;

  EXPECT_TRUE(send(uut, state, down(kFalseSquare + Vec2{100, 0})));

  EXPECT_TRUE(valueOf(state, FullID{1, 1}));
}
