#include "editor/tools/completion_modal.hpp"

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/tools/tool.hpp"
#include "recording_renderer.hpp"
#include "tool_harness.hpp"

// A centered modal over the middle half of its bounds; a press outside or Escape closes it.

namespace {

  using fluir::editor::CommentOption;
  using fluir::editor::CompletionModal;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::FunctionDefOption;
  using fluir::editor::InputEvent;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::key;
  using testutil::move;

  const EditorContext kCtx;
  const Rect kBounds{0, 0, 800, 600};
  const Rect kFrame{200, 150, 400, 300};

  struct Fixture {
    EditorState state{kCtx};
    CompletionModal uut{{{"Function", FunctionDefOption{}}, {"Comment", CommentOption{}}}, kBounds, kCtx.layout};
  };

}  // namespace

TEST(CompletionModal, TheFrameIsTheMiddleHalfOfTheBounds) {
  Fixture f;

  testutil::expectRectNear(f.uut.frame(), kFrame);
}

TEST(CompletionModal, APressOutsideClosesWithAnyButton) {
  Fixture f;

  EXPECT_FALSE(f.uut.onEvent(down(Vec2{100, 100}), f.state));
  EXPECT_FALSE(f.uut.onEvent(down(Vec2{700, 500}, InputEvent::Button::Right), f.state));
  EXPECT_FALSE(f.uut.onEvent(down(Vec2{400, 500}, InputEvent::Button::Middle), f.state));
}

TEST(CompletionModal, EscapeCloses) {
  Fixture f;

  EXPECT_FALSE(f.uut.onEvent(key(InputEvent::Key::Escape), f.state));
}

TEST(CompletionModal, PressesInsideMovesAndOtherKeysKeepItOpen) {
  Fixture f;

  EXPECT_TRUE(f.uut.onEvent(down(Vec2{210, 160}), f.state)) << "on the first row";
  EXPECT_TRUE(f.uut.onEvent(down(Vec2{400, 440}), f.state)) << "on empty panel";
  EXPECT_TRUE(f.uut.onEvent(move(Vec2{10, 10}), f.state));
  EXPECT_TRUE(f.uut.onEvent(testutil::up(Vec2{10, 10}), f.state));
  EXPECT_TRUE(f.uut.onEvent(key(InputEvent::Key::Delete), f.state));
  EXPECT_TRUE(f.uut.onEvent(testutil::text("x"), f.state));
}

TEST(CompletionModal, DrawShowsEachLabelInsideTheFrame) {
  Fixture f;
  testutil::RecordingRenderer r;

  f.uut.draw(r, kCtx);

  EXPECT_TRUE(testutil::hasFill(r.calls, kFrame));
  const auto texts = testutil::textStrings(r.calls);
  EXPECT_NE(std::ranges::find(texts, "Function"), texts.end());
  EXPECT_NE(std::ranges::find(texts, "Comment"), texts.end());
  for (const auto& call : testutil::opsOf(r.calls, testutil::DrawCall::Op::Text)) {
    EXPECT_TRUE(kFrame.contains(call.a)) << call.text;
  }
  EXPECT_EQ(f.uut.labels(), (std::vector<std::string>{"Function", "Comment"}));
}
