#include "editor/core/header_bar.hpp"

#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "recording_renderer.hpp"

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::HeaderBar;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using testutil::hasFill;
  using testutil::hasTextAt;
  using testutil::RecordingRenderer;
  using testutil::textStrings;

}  // namespace

TEST(HeaderBar, DrawShowsBarFillAndProgramName) {
  EditorContext ctx;
  ctx.program = std::filesystem::path("some/dir/example.fl");
  RecordingRenderer renderer;
  const Vec2 outputSize{800, 600};

  HeaderBar header([] {});
  header.draw(ctx, renderer, outputSize);

  EXPECT_TRUE(hasFill(renderer.calls, Rect{0, 0, outputSize.x, ctx.layout.chromeHeaderPx}));
  EXPECT_TRUE(hasTextAt(renderer.calls, "example.fl", Vec2{ctx.layout.textPad, ctx.layout.textPad}));
}

TEST(HeaderBar, SkipsNameWhenProgramUnset) {
  EditorContext ctx;  // ctx.program left unset
  RecordingRenderer renderer;
  const Vec2 outputSize{800, 600};

  HeaderBar header([] {});
  header.draw(ctx, renderer, outputSize);

  EXPECT_TRUE(hasFill(renderer.calls, Rect{0, 0, outputSize.x, ctx.layout.chromeHeaderPx}));
  // Only the Exit button's own label should be drawn; no filename text.
  EXPECT_EQ(textStrings(renderer.calls), std::vector<std::string>{"Exit"});
}

TEST(HeaderBar, ExitButtonLandsInTopRightCorner) {
  EditorContext ctx;
  RecordingRenderer renderer;
  const Vec2 outputSize{800, 600};

  HeaderBar header([] {});
  header.draw(ctx, renderer, outputSize);

  const Rect bounds = header.exitButton().bounds();
  EXPECT_NEAR(bounds.y, ctx.layout.textPad, 1e-6);
  EXPECT_NEAR(bounds.x + bounds.w, outputSize.x - ctx.layout.textPad, 1e-6);
  EXPECT_GT(bounds.x, 0.0);
  EXPECT_LT(bounds.x + bounds.w, outputSize.x + 1e-6);
}

TEST(HeaderBar, ClickOnButtonInvokesActionAndConsumes) {
  bool clicked = false;
  EditorContext ctx;
  RecordingRenderer renderer;
  const Vec2 outputSize{800, 600};

  HeaderBar header([&clicked] { clicked = true; });
  header.draw(ctx, renderer, outputSize);

  const bool consumed = header.handleClick(header.exitButton().bounds().center());

  EXPECT_TRUE(consumed);
  EXPECT_TRUE(clicked);
}

TEST(HeaderBar, ClickElsewhereDoesNotConsume) {
  bool clicked = false;
  EditorContext ctx;
  RecordingRenderer renderer;
  const Vec2 outputSize{800, 600};

  HeaderBar header([&clicked] { clicked = true; });
  header.draw(ctx, renderer, outputSize);

  ASSERT_LT(0.0, header.exitButton().bounds().x) << "margin must keep the button away from the origin";
  const bool consumed = header.handleClick(Vec2{0, 0});

  EXPECT_FALSE(consumed);
  EXPECT_FALSE(clicked);
}

TEST(HeaderBar, ClickBeforeAnyDrawDoesNotConsume) {
  bool clicked = false;
  HeaderBar header([&clicked] { clicked = true; });

  const bool consumed = header.handleClick(Vec2{0, 0});

  EXPECT_FALSE(consumed);
  EXPECT_FALSE(clicked);
}
