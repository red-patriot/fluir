#include "editor/gesture/inline_edit.hpp"

#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"

// InlineEdit owns the open/closed state of an actor's in-place editor: a null
// field is the closed state, and the owner supplies prefill and validation.

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::InlineEdit;
  using fluir::editor::TextField;

  constexpr double kGlyphPx = 8.0;  // matches TextField's fixed debug-font cell

  /** Always offers `text`; records how many times the owner was asked. */
  InlineEdit::Prefill fixedPrefill(std::string text, int& calls) {
    return [text, &calls]() -> std::optional<std::string> {
      ++calls;
      return text;
    };
  }

  InlineEdit::Prefill declinedPrefill() {
    return []() -> std::optional<std::string> { return std::nullopt; };
  }

  /** Records every draft it is asked to validate; always returns `accept`. */
  TextField::Commit recordingCommit(bool accept, std::vector<std::string>& seen) {
    return [accept, &seen](const EditorContext&, const std::string& text) {
      seen.push_back(text);
      return accept;
    };
  }

}  // namespace

TEST(InlineEdit, StartsClosed) {
  int prefillCalls = 0;
  InlineEdit edit(fixedPrefill("42", prefillCalls), TextField::Commit{});

  EXPECT_FALSE(edit.active());
  EXPECT_EQ(edit.field(), nullptr);
  EXPECT_EQ(prefillCalls, 0);
}

TEST(InlineEdit, BeginPrefillsTheDraftFromTheOwner) {
  int prefillCalls = 0;
  InlineEdit edit(fixedPrefill("42", prefillCalls), TextField::Commit{});

  EXPECT_TRUE(edit.begin(0.0));

  EXPECT_EQ(prefillCalls, 1);
  ASSERT_TRUE(edit.active());
  EXPECT_EQ(edit.field()->text(), "42");
  EXPECT_EQ(edit.field()->caret(), 0u);
}

TEST(InlineEdit, BeginPlacesTheCaretAtTheNearestGapToTheClick) {
  int prefillCalls = 0;
  InlineEdit edit(fixedPrefill("ab", prefillCalls), TextField::Commit{});

  edit.begin(kGlyphPx * 0.6);  // past the middle of 'a'

  ASSERT_TRUE(edit.active());
  EXPECT_EQ(edit.field()->caret(), 1u);
}

TEST(InlineEdit, BeginIsDeclinedWhenTheOwnerHasNothingEditable) {
  InlineEdit edit(declinedPrefill(), TextField::Commit{});

  EXPECT_FALSE(edit.begin(0.0));
  EXPECT_FALSE(edit.active());
}

TEST(InlineEdit, BeginOnAnOpenDraftOnlyReAimsTheCaret) {
  int prefillCalls = 0;
  InlineEdit edit(fixedPrefill("ab", prefillCalls), TextField::Commit{});
  ASSERT_TRUE(edit.begin(0.0));
  edit.field()->insert("z");  // the draft is now "zab", the model still "ab"

  EXPECT_TRUE(edit.begin(kGlyphPx * 2));

  EXPECT_EQ(prefillCalls, 1) << "the owner is not asked again";
  EXPECT_EQ(edit.field()->text(), "zab") << "the draft survives";
  EXPECT_EQ(edit.field()->caret(), 2u);
}

TEST(InlineEdit, EndClosesTheDraftWithoutRunningTheValidator) {
  int prefillCalls = 0;
  std::vector<std::string> seen;
  InlineEdit edit(fixedPrefill("abc", prefillCalls), recordingCommit(true, seen));
  ASSERT_TRUE(edit.begin(0.0));

  edit.end();

  EXPECT_FALSE(edit.active());
  EXPECT_EQ(edit.field(), nullptr);
  EXPECT_TRUE(seen.empty());
}

TEST(InlineEdit, ReopeningAfterEndPrefillsFromTheOwnerAgain) {
  int prefillCalls = 0;
  InlineEdit edit(fixedPrefill("42", prefillCalls), TextField::Commit{});
  ASSERT_TRUE(edit.begin(0.0));
  edit.field()->insert("9");
  edit.end();

  ASSERT_TRUE(edit.begin(0.0));

  EXPECT_EQ(prefillCalls, 2);
  EXPECT_EQ(edit.field()->text(), "42") << "the abandoned draft is gone";
}

TEST(InlineEdit, ARejectedCommitLeavesTheDraftOpenForTheOwnerToKeep) {
  int prefillCalls = 0;
  std::vector<std::string> seen;
  InlineEdit edit(fixedPrefill("abc", prefillCalls), recordingCommit(false, seen));
  ASSERT_TRUE(edit.begin(0.0));

  const EditorContext ctx;
  EXPECT_FALSE(edit.field()->commit(ctx));

  EXPECT_TRUE(edit.active());
  EXPECT_TRUE(edit.field()->invalid());
  EXPECT_EQ(edit.field()->text(), "abc");
}
