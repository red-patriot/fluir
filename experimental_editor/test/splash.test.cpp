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

  InputEvent mouseDown(InputEvent::Button button, Vec2 pos) {
    InputEvent ie;
    ie.type = InputEvent::Type::MouseDown;
    ie.button = button;
    ie.pos = pos;
    return ie;
  }

  InputEvent quit() {
    InputEvent ie;
    ie.type = InputEvent::Type::Quit;
    return ie;
  }

}  // namespace

TEST(SplashPage, DrawsOpenButton) {
  EditorContext ctx;
  RecordingRenderer renderer;
  SplashPage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);
  ASSERT_EQ(page.draw(), 0);

  const Vec2 textPos = page.openButton().bounds().topLeft() + Vec2{ctx.layout.textPad, ctx.layout.textPad};
  EXPECT_TRUE(hasTextAt(renderer.calls, "Open", textPos));
  EXPECT_TRUE(hasFill(renderer.calls, page.openButton().bounds()));
}

TEST(SplashPage, ClickingOpenButtonDoesNotCrash) {
  EditorContext ctx;
  RecordingRenderer renderer;
  SplashPage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);
  ASSERT_EQ(page.draw(), 0);

  const Vec2 center = page.openButton().bounds().center();
  EXPECT_EQ(page.update({mouseDown(InputEvent::Button::Left, center)}), 0);
  SUCCEED();  // no-op action -- the only thing to verify is that dispatch doesn't crash and returns cleanly.
}

TEST(SplashPage, NextStaysNullUntilOpenIsWired) {
  EditorContext ctx;
  RecordingRenderer renderer;
  SplashPage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  EXPECT_EQ(page.next(), nullptr) << "Open is a no-op this plan -- transition lands with the file-dialog step";
}

TEST(SplashPage, QuitEventStopsTheApp) {
  EditorContext ctx;
  RecordingRenderer renderer;
  SplashPage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  EXPECT_EQ(page.update({quit()}), 0);
  EXPECT_FALSE(ctx.running) << "Quit must reach SplashPage -- it is the app's entry page and never transitions";
}

TEST(SplashPage, ButtonRecentersOnResize) {
  EditorContext ctx;
  RecordingRenderer renderer;
  SplashPage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);
  ASSERT_EQ(page.draw(), 0);
  const Rect before = page.openButton().bounds();

  renderer.outputSize_ = Vec2{1200, 900};
  ASSERT_EQ(page.draw(), 0);
  const Rect after = page.openButton().bounds();

  EXPECT_NE(before, after) << "the button must re-layout each frame, not stay frozen from construction";
  EXPECT_EQ(after.center(), (Vec2{600, 450}));
}
