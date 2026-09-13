#include "editor/pages/splash.hpp"

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/input.hpp"
#include "recording_renderer.hpp"

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::InputEvent;
  using fluir::editor::Rect;
  using fluir::editor::SplashPage;
  using fluir::editor::Vec2;
  using testutil::hasFill;
  using testutil::hasTextAt;
  using testutil::RecordingRenderer;

}  // namespace

TEST(SplashPage, DrawsTheOpenButton) {
  EditorContext ctx;
  RecordingRenderer renderer;
  SplashPage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);
  ASSERT_EQ(page.draw(), 0);

  const Rect open = page.openButtonRect();
  EXPECT_TRUE(hasTextAt(renderer.calls, "Open", open.topLeft() + Vec2{ctx.layout.textPad, ctx.layout.textPad}));
  EXPECT_TRUE(hasFill(renderer.calls, open));
}

TEST(SplashPage, NextStaysNullUntilAFileIsOpened) {
  EditorContext ctx;
  RecordingRenderer renderer;
  SplashPage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  EXPECT_EQ(page.next(), nullptr);
}

TEST(SplashPage, QuitStopsTheApp) {
  EditorContext ctx;
  RecordingRenderer renderer;
  SplashPage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  EXPECT_EQ(page.update({InputEvent{.type = InputEvent::Type::Quit}}), 0);
  EXPECT_FALSE(ctx.running);
}

TEST(SplashPage, EscapeDoesNotStopTheApp) {
  EditorContext ctx;
  RecordingRenderer renderer;
  SplashPage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  EXPECT_EQ(page.update({InputEvent{.type = InputEvent::Type::KeyDown, .key = InputEvent::Key::Escape}}), 0);
  EXPECT_TRUE(ctx.running);
}

TEST(SplashPage, TheButtonRecentersOnResize) {
  EditorContext ctx;
  RecordingRenderer renderer;
  SplashPage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);
  const Rect before = page.openButtonRect();

  renderer.outputSize_ = Vec2{1200, 900};
  ASSERT_EQ(page.update({InputEvent{.type = InputEvent::Type::Resize}}), 0);

  EXPECT_NE(page.openButtonRect(), before);
  EXPECT_EQ(page.openButtonRect().center(), (Vec2{600, 450}));
}
