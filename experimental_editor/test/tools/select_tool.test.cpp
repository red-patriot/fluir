#include "editor/tools/select_tool.hpp"

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "tool_harness.hpp"

// simple_binary_expr.fl: function 1 {50,50,500,500}; binary 1 {125,85,25,25} with move grip {130,90,15,15}.

namespace {

  using fluir::FullID;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::InputEvent;
  using fluir::editor::SelectTool;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::send;

  const EditorContext kCtx;

}  // namespace

TEST(SelectTool, APressSelectsWhatItHitsWithoutConsuming) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");
  SelectTool uut;

  EXPECT_FALSE(send(uut, state, down(Vec2{127, 107})));
  EXPECT_EQ(state.selection, (FullID{1, 1}));

  EXPECT_FALSE(send(uut, state, down(Vec2{300, 300})));
  EXPECT_EQ(state.selection, (FullID{1}));
}

TEST(SelectTool, APressOnAGripSelectsItsOwner) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");
  SelectTool uut;

  send(uut, state, down(Vec2{137, 97}));

  EXPECT_EQ(state.selection, (FullID{1, 1}));
}

TEST(SelectTool, AMissClearsTheSelection) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");
  SelectTool uut;
  send(uut, state, down(Vec2{127, 107}));

  send(uut, state, down(Vec2{-10, -10}));

  EXPECT_FALSE(state.selection.has_value());
}

TEST(SelectTool, OtherButtonsLeaveTheSelectionAlone) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");
  SelectTool uut;
  send(uut, state, down(Vec2{127, 107}));

  send(uut, state, down(Vec2{-10, -10}, InputEvent::Button::Middle));

  EXPECT_EQ(state.selection, (FullID{1, 1}));
}

TEST(SelectTool, SelectionHonoursTheViewport) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/simple_binary_expr.fl");
  state.view.pan = Vec2{100, 0};
  SelectTool uut;

  send(uut, state, down(Vec2{227, 107}));  // world (127,107)

  EXPECT_EQ(state.selection, (FullID{1, 1}));
}
