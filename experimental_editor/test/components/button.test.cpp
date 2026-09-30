#include "editor/components/button.hpp"

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/input.hpp"
#include "recording_renderer.hpp"
#include "tool_harness.hpp"

namespace {

  using fluir::editor::ButtonComp;
  using fluir::editor::ButtonOptions;
  using fluir::editor::EditorContext;
  using fluir::editor::InputEvent;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::hasFill;
  using testutil::hasFillColored;
  using testutil::hasRect;
  using testutil::hasTextAt;
  using testutil::key;
  using testutil::move;
  using testutil::RecordingRenderer;
  using testutil::textStrings;
  using testutil::up;

  const EditorContext ctx;
  const Rect BUTTON_RECT{100, 100, 120, 30};
  const Vec2 INSIDE = BUTTON_RECT.center();
  const Vec2 OUTSIDE{BUTTON_RECT.x - 10, BUTTON_RECT.y - 10};

  // A placed button counting its clicks, enabled while `on` is true.
  struct Fixture {
    int clicks = 0;
    bool is_on = true;
    ButtonComp uut{
      ButtonOptions{.label = "Open", .onClick = [this]() { ++clicks; }, .enabled = [this]() { return is_on; }}};

    Fixture() { uut.place(BUTTON_RECT); }
  };

}  // namespace

TEST(Button, IsEnabledIfUnspecified) {
  const ButtonComp uut{ButtonOptions{.label = "Save"}};

  EXPECT_TRUE(uut.enabled());
}

TEST(Button, IsEnabledFollowsPredicate) {
  Fixture f;

  EXPECT_TRUE(f.uut.enabled());
  f.is_on = false;
  EXPECT_FALSE(f.uut.enabled());
}

TEST(Button, CanSpecifyRect) {
  ButtonComp uut{ButtonOptions{.label = "Save"}};

  uut.place(BUTTON_RECT);

  EXPECT_EQ(uut.rect(), BUTTON_RECT);
}

TEST(Button, IgnoresPressesIfUnplaced) {
  int clicks = 0;
  ButtonComp uut{ButtonOptions{.label = "Save", .onClick = [&] { ++clicks; }}};

  EXPECT_FALSE(uut.handle(down({0, 0})));
  EXPECT_FALSE(uut.handle(up({0, 0})));
  EXPECT_EQ(clicks, 0);
}

TEST(Button, RePlacingMovesHitArea) {
  Fixture f;
  const Rect moved{400, 300, BUTTON_RECT.w, BUTTON_RECT.h};

  f.uut.place(moved);

  EXPECT_FALSE(f.uut.handle(down(INSIDE)));
  EXPECT_TRUE(f.uut.handle(down(moved.center())));
  EXPECT_TRUE(f.uut.handle(up(moved.center())));
  EXPECT_EQ(f.clicks, 1);
}

TEST(Button, PreferredSizeFitsTheLabel) {
  RecordingRenderer renderer;
  const ButtonComp uut{ButtonOptions{.label = "Save"}};

  const Vec2 size = uut.preferredSize(ctx.layout.textPad, renderer);
  const Vec2 text = renderer.measureText("Save");

  EXPECT_GT(size.x, text.x);
  EXPECT_GT(size.y, text.y);
}

TEST(Button, LongerLabelIsWider) {
  RecordingRenderer renderer;
  const ButtonComp shortLabel{ButtonOptions{.label = "Go"}};
  const ButtonComp longLabel{ButtonOptions{.label = "Go somewhere else"}};

  EXPECT_GT(longLabel.preferredSize(ctx.layout.textPad, renderer).x,
            shortLabel.preferredSize(ctx.layout.textPad, renderer).x);
}

TEST(Button, MorePaddingIsLarger) {
  RecordingRenderer renderer;
  const ButtonComp uut{ButtonOptions{.label = "Save"}};
  const double pad = ctx.layout.textPad;

  const Vec2 small = uut.preferredSize(pad, renderer);
  const Vec2 large = uut.preferredSize(pad * 2, renderer);

  EXPECT_GT(large.x, small.x);
  EXPECT_GT(large.y, small.y);
}

TEST(Button, PressAndReleaseInsideClicksOnce) {
  Fixture f;

  EXPECT_TRUE(f.uut.handle(down(INSIDE)));
  EXPECT_TRUE(f.uut.handle(up(INSIDE)));
  EXPECT_EQ(f.clicks, 1);
}

TEST(Button, PressDoesNotClick) {
  Fixture f;

  f.uut.handle(down(INSIDE));

  EXPECT_EQ(f.clicks, 0);
}

TEST(Button, ReleaseOutsideAfterPressDoesNotClick) {
  Fixture f;

  f.uut.handle(down(INSIDE));

  EXPECT_TRUE(f.uut.handle(up(OUTSIDE)));
  EXPECT_EQ(f.clicks, 0);
}

TEST(Button, DragOffThenLaterReleaseInsideDoesNotClick) {
  Fixture f;

  f.uut.handle(down(INSIDE));
  f.uut.handle(up(OUTSIDE));

  EXPECT_FALSE(f.uut.handle(up(INSIDE)));
  EXPECT_EQ(f.clicks, 0);
}

TEST(Button, PressOutsideThenReleaseInsideIsIgnored) {
  Fixture f;

  EXPECT_FALSE(f.uut.handle(down(OUTSIDE)));
  EXPECT_FALSE(f.uut.handle(up(INSIDE)));
  EXPECT_EQ(f.clicks, 0);
}

TEST(Button, MovingWhilePressedStillClicksOnRelease) {
  Fixture f;

  f.uut.handle(down(INSIDE));
  f.uut.handle(move(INSIDE));
  f.uut.handle(move(OUTSIDE));
  f.uut.handle(move(INSIDE));
  f.uut.handle(up(INSIDE));

  EXPECT_EQ(f.clicks, 1);
}

TEST(Button, OtherMouseButtonsAreIgnored) {
  Fixture f;

  for (const auto button : {InputEvent::Button::Right, InputEvent::Button::Middle}) {
    EXPECT_FALSE(f.uut.handle(down(INSIDE, button)));
    EXPECT_FALSE(f.uut.handle(up(INSIDE, button)));
  }
  EXPECT_EQ(f.clicks, 0);
}

TEST(Button, MovesAndKeysAreIgnored) {
  Fixture f;

  EXPECT_FALSE(f.uut.handle(move(INSIDE)));
  EXPECT_FALSE(f.uut.handle(key(InputEvent::Key::Return)));
  EXPECT_FALSE(f.uut.handle(key(InputEvent::Key::Space)));
  EXPECT_EQ(f.clicks, 0);
}

TEST(Button, EmptyOnClickIsANoOp) {
  ButtonComp uut{ButtonOptions{.label = "Save"}};
  uut.place(BUTTON_RECT);

  EXPECT_TRUE(uut.handle(down(INSIDE)));
  EXPECT_TRUE(uut.handle(up(INSIDE)));
}

TEST(Button, DisabledReturnsWhatItWouldHaveButDoesNotClick) {
  Fixture f;
  f.is_on = false;

  EXPECT_TRUE(f.uut.handle(down(INSIDE)));
  EXPECT_TRUE(f.uut.handle(up(INSIDE)));
  EXPECT_EQ(f.clicks, 0);
}

TEST(Button, DisabledAtReleaseDoesNotClick) {
  Fixture f;

  f.uut.handle(down(INSIDE));
  f.is_on = false;
  f.uut.handle(up(INSIDE));

  EXPECT_EQ(f.clicks, 0);
}

TEST(Button, EnabledAtReleaseClicks) {
  Fixture f;
  f.is_on = false;

  f.uut.handle(down(INSIDE));
  f.is_on = true;
  f.uut.handle(up(INSIDE));

  EXPECT_EQ(f.clicks, 1);
}

TEST(Button, DrawFillsAndBordersItsRect) {
  Fixture f;
  RecordingRenderer renderer;

  f.uut.draw(renderer, ctx.theme);

  EXPECT_TRUE(hasFill(renderer.calls, BUTTON_RECT));
  EXPECT_TRUE(hasRect(renderer.calls, BUTTON_RECT));
}

TEST(Button, DrawCentersTheLabel) {
  Fixture f;
  RecordingRenderer renderer;

  f.uut.draw(renderer, ctx.theme);

  EXPECT_EQ(textStrings(renderer.calls), std::vector<std::string>{"Open"});
  EXPECT_TRUE(hasTextAt(renderer.calls, "Open", BUTTON_RECT.center() - renderer.measureText("Open") / 2));
}

TEST(Button, DrawFillsByEnabledState) {
  Fixture f;
  RecordingRenderer enabled;
  RecordingRenderer disabled;

  f.uut.draw(enabled, ctx.theme);
  f.is_on = false;
  f.uut.draw(disabled, ctx.theme);

  EXPECT_TRUE(hasFillColored(enabled.calls, BUTTON_RECT, ctx.theme.buttonEnabled));
  EXPECT_FALSE(hasFillColored(enabled.calls, BUTTON_RECT, ctx.theme.buttonDisabled));
  EXPECT_TRUE(hasFillColored(disabled.calls, BUTTON_RECT, ctx.theme.buttonDisabled));
  EXPECT_FALSE(hasFillColored(disabled.calls, BUTTON_RECT, ctx.theme.buttonEnabled));
}
