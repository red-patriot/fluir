#include "editor/view/toolbar.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/components/button.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "recording_renderer.hpp"
#include "tool_harness.hpp"

// Buttons size themselves from their labels and row left or right. Contracts
// here are placement, drawing and click routing, not pixel counts.

namespace {

  using fluir::editor::ButtonComp;
  using fluir::editor::ButtonOptions;
  using fluir::editor::drawToolbar;
  using fluir::editor::EditorContext;
  using fluir::editor::handleToolbar;
  using fluir::editor::layoutToolbar;
  using fluir::editor::Rect;
  using fluir::editor::Toolbar;
  using fluir::editor::ToolbarLayout;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::hasFill;
  using testutil::hasTextAt;
  using testutil::RecordingRenderer;
  using testutil::textStrings;
  using testutil::up;

  constexpr double kWidth = 800;
  const EditorContext kCtx;

  ButtonComp button(std::string label, std::function<void()> onClick = [] {}) {
    return ButtonComp{ButtonOptions{.label = std::move(label), .onClick = std::move(onClick)}};
  }

  // Save, Save As, Undo left; Exit right.
  Toolbar headerLike() {
    return Toolbar{.left = {button("Save"), button("Save As"), button("Undo")}, .right = {button("Exit")}, .label = {}};
  }

  bool hasText(const std::vector<testutil::DrawCall>& calls, const std::string& want) {
    const auto texts = textStrings(calls);
    return std::find(texts.begin(), texts.end(), want) != texts.end();
  }

}  // namespace

TEST(Toolbar, DrawShowsTheBarFillAndTheLabelAfterTheLastLeftButton) {
  RecordingRenderer r;
  Toolbar bar = headerLike();
  bar.label = "example.fl";
  const ToolbarLayout layout = layoutToolbar(bar, kWidth, kCtx.layout, r);

  drawToolbar(r, bar, layout, kCtx);

  const Rect undo = bar.left[2].rect();
  EXPECT_TRUE(hasFill(r.calls, Rect{0, 0, kWidth, kCtx.layout.chromeHeaderPx}));
  EXPECT_TRUE(hasTextAt(r.calls, "example.fl", Vec2{undo.x + undo.w + kCtx.layout.textPad, kCtx.layout.textPad}));
}

TEST(Toolbar, DrawSkipsTheLabelWhenEmpty) {
  RecordingRenderer r;
  Toolbar bar = headerLike();
  const ToolbarLayout layout = layoutToolbar(bar, kWidth, kCtx.layout, r);

  drawToolbar(r, bar, layout, kCtx);

  EXPECT_TRUE(hasText(r.calls, "Save"));
  EXPECT_TRUE(hasText(r.calls, "Exit"));
  EXPECT_EQ(textStrings(r.calls).size(), bar.left.size() + bar.right.size());
}

TEST(Toolbar, RightButtonsSitInTheTopRightCorner) {
  RecordingRenderer r;
  Toolbar bar = headerLike();
  layoutToolbar(bar, kWidth, kCtx.layout, r);

  const Rect exit = bar.right[0].rect();
  EXPECT_NEAR(exit.y, kCtx.layout.textPad, 1e-6);
  EXPECT_NEAR(exit.x + exit.w, kWidth - kCtx.layout.textPad, 1e-6);
}

TEST(Toolbar, RightButtonsKeepListOrder) {
  RecordingRenderer r;
  Toolbar bar = headerLike();
  bar.right.push_back(button("Quit"));
  layoutToolbar(bar, kWidth, kCtx.layout, r);

  const Rect exit = bar.right[0].rect();
  const Rect quit = bar.right[1].rect();
  EXPECT_LT(exit.x + exit.w, quit.x);
  EXPECT_NEAR(quit.x + quit.w, kWidth - kCtx.layout.textPad, 1e-6);
}

TEST(Toolbar, LeftButtonsRowFromTheTopLeftInOrder) {
  RecordingRenderer r;
  Toolbar bar = headerLike();
  layoutToolbar(bar, kWidth, kCtx.layout, r);

  const Rect save = bar.left[0].rect();
  EXPECT_NEAR(save.x, kCtx.layout.textPad, 1e-6);
  EXPECT_NEAR(save.y, kCtx.layout.textPad, 1e-6);
  EXPECT_GT(bar.left[1].rect().x, save.x + save.w);
}

TEST(Toolbar, ButtonsSizeThemselvesFromTheirLabels) {
  RecordingRenderer r;
  Toolbar bar = headerLike();
  layoutToolbar(bar, kWidth, kCtx.layout, r);

  EXPECT_GT(bar.left[1].rect().w, bar.left[0].rect().w) << "\"Save As\" is a longer label than \"Save\"";
  EXPECT_GT(bar.left[0].rect().w, r.measureText("Save").x);
}

TEST(Toolbar, AddingALeftButtonShiftsTheOthersButNotTheRightOnes) {
  RecordingRenderer r;
  Toolbar before = headerLike();
  Toolbar after = headerLike();
  after.left.insert(after.left.begin(), button("New"));

  layoutToolbar(before, kWidth, kCtx.layout, r);
  layoutToolbar(after, kWidth, kCtx.layout, r);

  EXPECT_GT(after.left[1].rect().x, before.left[0].rect().x) << "the new button pushes the rest right";
  EXPECT_EQ(after.right[0].rect(), before.right[0].rect()) << "right buttons stay flush to the edge";
}

TEST(Toolbar, HandleClicksTheButtonUnderAPressAndRelease) {
  RecordingRenderer r;
  int exits = 0;
  Toolbar bar = headerLike();
  bar.right[0] = button("Exit", [&] { ++exits; });
  layoutToolbar(bar, kWidth, kCtx.layout, r);
  const Vec2 at = bar.right[0].rect().center();

  EXPECT_TRUE(handleToolbar(bar, down(at)));
  EXPECT_TRUE(handleToolbar(bar, up(at)));
  EXPECT_EQ(exits, 1);
}

TEST(Toolbar, HandleIgnoresEventsOffEveryButton) {
  RecordingRenderer r;
  int clicks = 0;
  Toolbar bar{.left = {button("Save", [&] { ++clicks; })}, .right = {button("Exit", [&] { ++clicks; })}, .label = {}};
  layoutToolbar(bar, kWidth, kCtx.layout, r);
  const Vec2 off{kWidth / 2, 500};

  EXPECT_FALSE(handleToolbar(bar, down(off)));
  EXPECT_FALSE(handleToolbar(bar, up(off)));
  EXPECT_EQ(clicks, 0);
}
