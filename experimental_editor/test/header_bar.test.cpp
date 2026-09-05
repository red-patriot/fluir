#include "../include/editor/pages/header_bar.hpp"

#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "recording_renderer.hpp"

namespace {

  using fluir::editor::Actor;
  using fluir::editor::EditorContext;
  using fluir::editor::HeaderBar;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using testutil::hasFill;
  using testutil::hasTextAt;
  using testutil::RecordingRenderer;
  using testutil::textStrings;

}  // namespace

TEST(HeaderBar, DrawChromeShowsBarFillAndProgramName) {
  EditorContext ctx;
  ctx.program = std::filesystem::path("some/dir/example.fl");
  RecordingRenderer renderer;
  const Vec2 outputSize{800, 600};

  HeaderBar header([] {});
  header.drawChrome(ctx, renderer, outputSize);

  EXPECT_TRUE(hasFill(renderer.calls, Rect{0, 0, outputSize.x, ctx.layout.chromeHeaderPx}));
  EXPECT_TRUE(hasTextAt(renderer.calls, "example.fl", Vec2{ctx.layout.textPad, ctx.layout.textPad}));
}

TEST(HeaderBar, DrawChromeSkipsNameWhenProgramUnset) {
  EditorContext ctx;  // ctx.program left unset
  RecordingRenderer renderer;
  const Vec2 outputSize{800, 600};

  HeaderBar header([] {});
  header.drawChrome(ctx, renderer, outputSize);

  EXPECT_TRUE(hasFill(renderer.calls, Rect{0, 0, outputSize.x, ctx.layout.chromeHeaderPx}));
  // drawChrome no longer draws the button's own label -- no text at all.
  EXPECT_TRUE(textStrings(renderer.calls).empty());
}

TEST(HeaderBar, LayoutPositionsExitButtonInTopRightCorner) {
  EditorContext ctx;
  const Vec2 outputSize{800, 600};

  HeaderBar header([] {});
  header.layout(ctx, outputSize);

  const Rect bounds = header.exitButton().bounds();
  EXPECT_NEAR(bounds.y, ctx.layout.textPad, 1e-6);
  EXPECT_NEAR(bounds.x + bounds.w, outputSize.x - ctx.layout.textPad, 1e-6);
  EXPECT_GT(bounds.x, 0.0);
  EXPECT_LT(bounds.x + bounds.w, outputSize.x + 1e-6);
}

TEST(HeaderBar, ActorsReturnsExitButtonPointer) {
  HeaderBar header([] {});

  const std::vector<Actor*> actors = header.actors();

  ASSERT_EQ(actors.size(), 1u);
  EXPECT_EQ(actors[0], &header.exitButton());
}

TEST(HeaderBar, ActorsPointerReflectsLayoutAndInvokesAction) {
  bool clicked = false;
  EditorContext ctx;
  const Vec2 outputSize{800, 600};

  HeaderBar header([&clicked] { clicked = true; });
  header.layout(ctx, outputSize);

  Actor* button = header.actors().front();
  EXPECT_EQ(button->bounds(), header.exitButton().bounds());

  button->onClick(button->bounds().center());
  EXPECT_TRUE(clicked);
}

TEST(HeaderBar, ExitButtonZeroSizedBeforeAnyLayout) {
  HeaderBar header([] {});

  // Documents the placeholder pre-layout state: a point can never be "inside"
  // a zero-size Rect (Rect::contains requires strict less-than on the far edge).
  EXPECT_EQ(header.exitButton().bounds(), (Rect{0, 0, 0, 0}));
  EXPECT_FALSE(header.exitButton().bounds().contains(Vec2{0, 0}));
}
