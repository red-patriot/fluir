#include "editor/components/text_field.hpp"

#include <cstddef>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"
#include "recording_renderer.hpp"

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::InputEvent;
  using fluir::editor::Rect;
  using fluir::editor::Subview;
  using fluir::editor::TextField;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::hasFill;
  using testutil::hasTextAt;
  using testutil::RecordingRenderer;

  constexpr double kGlyphPx = 8.0;  // matches TextField's fixed debug-font cell

  /** A draft opened on `text`, with the caret where a click at `dxScreenPx` puts it. */
  TextField openField(std::string text, double dxScreenPx = 0.0, TextField::Commit commit = {}) {
    const std::size_t caret = TextField::indexAt(text, dxScreenPx);
    return TextField{std::move(text), caret, std::move(commit)};
  }

  /** Records every draft it is asked to validate; always returns `accept`. */
  TextField::Commit recordingCommit(bool accept, std::vector<std::string>& seen) {
    return [accept, &seen](const EditorContext&, const std::string& text) {
      seen.push_back(text);
      return accept;
    };
  }

}  // namespace

TEST(TextField, TypingInsertsAtTheCaret) {
  TextField field = openField("ac", kGlyphPx);  // caret at index 1, between 'a' and 'c'

  field.insert("b");

  EXPECT_EQ(field.text(), "abc");
  EXPECT_EQ(field.caret(), 2u);
}

TEST(TextField, InsertIgnoresControlCharacters) {
  TextField field = openField("", 0.0);

  field.insert("\n\t");
  EXPECT_EQ(field.text(), "");
  EXPECT_EQ(field.caret(), 0u);

  field.insert("a\tb");  // control characters within a mixed insert are dropped
  EXPECT_EQ(field.text(), "ab");
  EXPECT_EQ(field.caret(), 2u);
}

TEST(TextField, BackspaceDeletesBeforeTheCaretAndDeleteDeletesAfter) {
  TextField field = openField("abc", 0.0);
  field.onKey(InputEvent::Key::Right);  // caret at 1, between 'a' and 'b'

  EXPECT_TRUE(field.onKey(InputEvent::Key::Backspace));
  EXPECT_EQ(field.text(), "bc");
  EXPECT_EQ(field.caret(), 0u);

  EXPECT_TRUE(field.onKey(InputEvent::Key::Delete));
  EXPECT_EQ(field.text(), "c");
  EXPECT_EQ(field.caret(), 0u);
}

TEST(TextField, BackspaceAtTheStartAndDeleteAtTheEndLeaveTheDraftAlone) {
  TextField field = openField("ab", 0.0);  // caret at 0

  EXPECT_TRUE(field.onKey(InputEvent::Key::Backspace));
  EXPECT_EQ(field.text(), "ab");
  EXPECT_EQ(field.caret(), 0u);

  field.onKey(InputEvent::Key::End);  // caret at 2, the end
  EXPECT_TRUE(field.onKey(InputEvent::Key::Delete));
  EXPECT_EQ(field.text(), "ab");
  EXPECT_EQ(field.caret(), 2u);
}

TEST(TextField, LeftAndRightMoveTheCaretAndClampAtTheEnds) {
  TextField field = openField("ab", 0.0);  // caret at 0

  EXPECT_TRUE(field.onKey(InputEvent::Key::Left));
  EXPECT_EQ(field.caret(), 0u);  // clamped at the start

  EXPECT_TRUE(field.onKey(InputEvent::Key::Right));
  EXPECT_EQ(field.caret(), 1u);
  EXPECT_TRUE(field.onKey(InputEvent::Key::Right));
  EXPECT_EQ(field.caret(), 2u);
  EXPECT_TRUE(field.onKey(InputEvent::Key::Right));
  EXPECT_EQ(field.caret(), 2u);  // clamped at the end

  EXPECT_TRUE(field.onKey(InputEvent::Key::Left));
  EXPECT_EQ(field.caret(), 1u);
}

TEST(TextField, HomeAndEndJumpToTheDraftEnds) {
  TextField field = openField("abcd", 2 * kGlyphPx);  // caret starts in the middle

  EXPECT_TRUE(field.onKey(InputEvent::Key::Home));
  EXPECT_EQ(field.caret(), 0u);

  EXPECT_TRUE(field.onKey(InputEvent::Key::End));
  EXPECT_EQ(field.caret(), 4u);
}

TEST(TextField, CommitRunsTheValidatorOnce) {
  std::vector<std::string> seen;
  TextField field = openField("42", 0.0, recordingCommit(true, seen));

  const EditorContext ctx;
  EXPECT_TRUE(field.commit(ctx));

  ASSERT_EQ(seen.size(), 1u);
  EXPECT_EQ(seen[0], "42");
  EXPECT_FALSE(field.invalid());
}

TEST(TextField, ARejectedCommitMarksTheDraftInvalid) {
  std::vector<std::string> seen;
  TextField field = openField("abc", 0.0, recordingCommit(false, seen));

  const EditorContext ctx;
  EXPECT_FALSE(field.commit(ctx));

  EXPECT_TRUE(field.invalid());
  EXPECT_EQ(field.text(), "abc");
}

TEST(TextField, EditingAfterARejectionClearsTheInvalidMark) {
  std::vector<std::string> seen;
  TextField field = openField("abc", 0.0, recordingCommit(false, seen));
  const EditorContext ctx;
  field.commit(ctx);
  ASSERT_TRUE(field.invalid());

  field.insert("x");

  EXPECT_FALSE(field.invalid());
}

TEST(TextField, DrawsTheDraftAndACaretAtTheCaretIndex) {
  TextField field = openField("abc", 0.0);
  field.onKey(InputEvent::Key::Right);
  field.onKey(InputEvent::Key::Right);  // caret at 2

  const EditorContext ctx;
  RecordingRenderer renderer;
  const Viewport viewport;
  const Vec2 localTextPos{10, 20};
  {
    const Subview view{viewport, Rect{0, 0, 1000, 1000}, renderer};
    field.draw(view, ctx, localTextPos);
  }

  EXPECT_TRUE(hasTextAt(renderer.calls, "abc", localTextPos));
  EXPECT_TRUE(hasFill(renderer.calls, Rect{localTextPos.x + kGlyphPx * 2, localTextPos.y, 1.0, kGlyphPx}));
}

TEST(TextField, TheCaretTracksTheTextOriginAtAnyZoom) {
  TextField field = openField("abc", 0.0);
  field.onKey(InputEvent::Key::Right);  // caret at 1

  const EditorContext ctx;
  RecordingRenderer renderer;
  const Viewport viewport{.pan = {100, 50}, .scale = 2.0};
  const Vec2 localTextPos{10, 20};
  {
    const Subview view{viewport, Rect{0, 0, 1000, 1000}, renderer};
    field.draw(view, ctx, localTextPos);
  }

  // origin = localTextPos * scale + pan = (10*2+100, 20*2+50)
  const Vec2 origin{120, 90};
  EXPECT_TRUE(hasTextAt(renderer.calls, "abc", origin));
  // Caret geometry stays anchored at `origin` and in unscaled glyph px, not
  // multiplied by the viewport's scale.
  EXPECT_TRUE(hasFill(renderer.calls, Rect{origin.x + kGlyphPx, origin.y, 1.0, kGlyphPx}));
}
