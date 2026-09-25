#include "editor/tools/branch_toggle_tool.hpp"

#include <utility>
#include <variant>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/tree.hpp"
#include "editor/core/tree_path.hpp"
#include "tool_harness.hpp"

// Function 1 {0,0,500,500} (body from y 25) holds conditional 2 -> frame {10,35,100,100} and conditional 3 ->
// frame {10,175,100,100}. A header band is 25 tall and its tag reserves 29.6 px, so the 15 px arrow sits at
// {39.6,+5,15,15} inside each band: centres {47,47.5} and {47,187.5}.

namespace {

  using fluir::FullID;
  using fluir::editor::BranchToggleTool;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::ELSE_BRANCH_ID;
  using fluir::editor::InputEvent;
  using fluir::editor::THEN_BRANCH_ID;
  using fluir::editor::Vec2;
  namespace et = fluir::editor::et;
  using testutil::down;
  using testutil::send;

  const EditorContext kCtx;
  constexpr Vec2 kFirstArrow{47, 47.5};
  constexpr Vec2 kSecondArrow{47, 187.5};

  et::Conditional makeConditional(fluir::ID id, int y) {
    et::Conditional conditional;
    conditional.id = id;
    conditional.location = {.x = 2, .y = y, .z = 0, .width = 20, .height = 20};
    return conditional;
  }

  void loadTwoConditionals(EditorState& state) {
    et::FunctionDecl fn;
    fn.id = 1;
    fn.location = {.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    fn.body.nodes.emplace(2, makeConditional(2, 2));
    fn.body.nodes.emplace(3, makeConditional(3, 30));
    et::ParseTree tree;
    tree.declarations.emplace(1, et::Declaration{std::move(fn)});
    state.editor.load(std::move(tree));
  }

  fluir::ID shownBranch(const EditorState& state, const FullID& path) {
    return std::get<et::Conditional>(*fluir::editor::nodeAt(state.editor.tree(), path)).annotation.shownBranch;
  }

}  // namespace

TEST(BranchToggleTool, PressingTheArrowSwitchesTheShownBranchAndConsumes) {
  EditorState state{kCtx};
  loadTwoConditionals(state);
  BranchToggleTool uut;

  EXPECT_TRUE(send(uut, state, down(kFirstArrow)));
  EXPECT_EQ(shownBranch(state, FullID{1, 2}), ELSE_BRANCH_ID);

  EXPECT_TRUE(send(uut, state, down(kFirstArrow)));
  EXPECT_EQ(shownBranch(state, FullID{1, 2}), THEN_BRANCH_ID);
}

// The shown branch is view state, so switching it is not an edit.
TEST(BranchToggleTool, TogglingEntersNoUndoHistory) {
  EditorState state{kCtx};
  loadTwoConditionals(state);
  BranchToggleTool uut;

  send(uut, state, down(kFirstArrow));

  EXPECT_FALSE(state.editor.canUndo());
}

TEST(BranchToggleTool, PressingElsewhereInTheHeaderDoesNothing) {
  EditorState state{kCtx};
  loadTwoConditionals(state);
  BranchToggleTool uut;

  EXPECT_FALSE(send(uut, state, down(Vec2{20, 47.5})));  // over the tag, left of the arrow
  EXPECT_FALSE(send(uut, state, down(Vec2{80, 47.5})));  // header band, right of the arrow
  EXPECT_FALSE(send(uut, state, down(Vec2{47, 90})));    // inside the branch, below the band
  EXPECT_EQ(shownBranch(state, FullID{1, 2}), THEN_BRANCH_ID);
}

TEST(BranchToggleTool, AFunctionHeaderHasNoBranchToSwitch) {
  EditorState state{kCtx};
  loadTwoConditionals(state);
  BranchToggleTool uut;

  EXPECT_FALSE(send(uut, state, down(Vec2{47, 12.5})));
}

TEST(BranchToggleTool, OnlyALeftPressToggles) {
  EditorState state{kCtx};
  loadTwoConditionals(state);
  BranchToggleTool uut;

  EXPECT_FALSE(send(uut, state, down(kFirstArrow, InputEvent::Button::Right)));
  EXPECT_EQ(shownBranch(state, FullID{1, 2}), THEN_BRANCH_ID);
}

TEST(BranchToggleTool, TwoConditionalsSwitchIndependently) {
  EditorState state{kCtx};
  loadTwoConditionals(state);
  BranchToggleTool uut;

  EXPECT_TRUE(send(uut, state, down(kSecondArrow)));

  EXPECT_EQ(shownBranch(state, FullID{1, 3}), ELSE_BRANCH_ID);
  EXPECT_EQ(shownBranch(state, FullID{1, 2}), THEN_BRANCH_ID);
}

TEST(BranchToggleTool, PressesHonourTheViewport) {
  EditorState state{kCtx};
  loadTwoConditionals(state);
  state.view.pan = Vec2{100, 0};
  BranchToggleTool uut;

  EXPECT_TRUE(send(uut, state, down(kFirstArrow + Vec2{100, 0})));
  EXPECT_EQ(shownBranch(state, FullID{1, 2}), ELSE_BRANCH_ID);
}
