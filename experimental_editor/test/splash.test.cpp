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

  InputEvent quit() {
    InputEvent ie;
    ie.type = InputEvent::Type::Quit;
    return ie;
  }

  InputEvent resize() {
    InputEvent ie;
    ie.type = InputEvent::Type::Resize;
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

TEST(SplashPage, NextStaysNullUntilOpenIsWired) {
  EditorContext ctx;
  RecordingRenderer renderer;
  SplashPage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  EXPECT_EQ(page.next(), nullptr) << "next() is null until Open is clicked and a file is chosen";
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
  ASSERT_EQ(page.update({resize()}), 0);
  const Rect after = page.openButton().bounds();

  EXPECT_NE(before, after) << "a Resize event must re-layout the button, not leave it frozen";
  EXPECT_EQ(after.center(), (Vec2{600, 450}));
}
