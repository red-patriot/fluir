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

namespace {

  fluir::pt::Scope makeScope(fluir::ID id, int y, int h) {
    return fluir::pt::Scope{.id = id, .location = {.x = 0, .y = y, .z = 0, .width = 20, .height = h}, .body = {}};
  }

  // Function 1 {0,0,500,500} holds conditional 20 -> frame {10,35,100,90}, then branch content from
  // y 60 with constant 1 at {15,65,50,50}; else branch {10,95,100,30} is empty.
  fluir::pt::ParseTree conditionalTree() {
    fluir::pt::Scope then = makeScope(0, 0, 12);
    then.body.nodes.emplace(1,
                            fluir::pt::Constant{.id = 1,
                                                .location = {.x = 1, .y = 1, .z = 0, .width = 10, .height = 10},
                                                .value = fluir::literals_types::I32{0}});

    fluir::pt::FunctionDecl fn;
    fn.id = 1;
    fn.location = fluir::FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    fn.body.nodes.emplace(20,
                          fluir::pt::Conditional{.id = 20,
                                                 .location = {.x = 2, .y = 2, .z = 0, .width = 20, .height = 18},
                                                 .condition = {},
                                                 .inputs = {},
                                                 .outputs = {},
                                                 .thenScope = xyz::indirect{std::move(then)},
                                                 .elseScope = xyz::indirect{makeScope(1, 12, 6)}});

    fluir::pt::ParseTree tree;
    tree.declarations.emplace(1, fluir::pt::Declaration{std::move(fn)});
    return tree;
  }

}  // namespace

TEST(SelectTool, ANestedNodeSelectsAtItsOwnPath) {
  EditorState state{kCtx};
  state.editor.load(conditionalTree());
  SelectTool tool;

  testutil::send(tool, state, down(Vec2{20, 90}));

  ASSERT_TRUE(state.selection.has_value());
  EXPECT_EQ(*state.selection, (FullID{1, 20, 0, 1}));
}

// A press on empty branch space names the branch, which is what a later completion needs;
// the outline it draws is the conditional's.
TEST(SelectTool, AnEmptyBranchSelectsItsScopePath) {
  EditorState state{kCtx};
  state.editor.load(conditionalTree());
  SelectTool tool;

  testutil::send(tool, state, down(Vec2{90, 110}));

  ASSERT_TRUE(state.selection.has_value());
  EXPECT_EQ(*state.selection, (FullID{1, 20, 1}));
}
