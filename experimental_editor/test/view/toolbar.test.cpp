#include "editor/view/toolbar.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "recording_renderer.hpp"

// Buttons size themselves from their labels and row left or right. Contracts
// here are placement, drawing and hit-testing, not pixel counts.

namespace {

  using fluir::editor::Button;
  using fluir::editor::buttonAt;
  using fluir::editor::drawButton;
  using fluir::editor::drawToolbar;
  using fluir::editor::EditorContext;
  using fluir::editor::layoutToolbar;
  using fluir::editor::Rect;
  using fluir::editor::Toolbar;
  using fluir::editor::ToolbarLayout;
  using fluir::editor::Vec2;
  using testutil::hasFill;
  using testutil::hasRect;
  using testutil::hasTextAt;
  using testutil::RecordingRenderer;
  using testutil::textStrings;

  constexpr double kWidth = 800;
  const EditorContext kCtx;

  // Save, Save As, Undo left; Exit right.
  Toolbar headerLike() {
    return Toolbar{.buttons = {Button{.label = "Save", .onClick = [] {}},
                               Button{.label = "Save As", .onClick = [] {}},
                               Button{.label = "Undo", .onClick = [] {}},
                               Button{.label = "Exit", .onClick = [] {}, .align = Button::Align::Right}},
                   .label = {}};
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

  const Rect undo = layout.buttons[2];
  EXPECT_TRUE(hasFill(r.calls, Rect{0, 0, kWidth, kCtx.layout.chromeHeaderPx}));
  EXPECT_TRUE(hasTextAt(r.calls, "example.fl", Vec2{undo.x + undo.w + kCtx.layout.textPad, kCtx.layout.textPad}));
}

TEST(Toolbar, DrawSkipsTheLabelWhenEmpty) {
  RecordingRenderer r;
  const Toolbar bar = headerLike();
  const ToolbarLayout layout = layoutToolbar(bar, kWidth, kCtx.layout, r);

  drawToolbar(r, bar, layout, kCtx);

  EXPECT_TRUE(hasText(r.calls, "Save"));
  EXPECT_TRUE(hasText(r.calls, "Exit"));
  EXPECT_EQ(textStrings(r.calls).size(), bar.buttons.size());
}

TEST(Toolbar, RightAlignedButtonsSitInTheTopRightCorner) {
  RecordingRenderer r;
  const ToolbarLayout layout = layoutToolbar(headerLike(), kWidth, kCtx.layout, r);

  const Rect exit = layout.buttons[3];
  EXPECT_NEAR(exit.y, kCtx.layout.textPad, 1e-6);
  EXPECT_NEAR(exit.x + exit.w, kWidth - kCtx.layout.textPad, 1e-6);
}

TEST(Toolbar, LeftAlignedButtonsRowFromTheTopLeftInOrder) {
  RecordingRenderer r;
  const ToolbarLayout layout = layoutToolbar(headerLike(), kWidth, kCtx.layout, r);

  EXPECT_NEAR(layout.buttons[0].x, kCtx.layout.textPad, 1e-6);
  EXPECT_NEAR(layout.buttons[0].y, kCtx.layout.textPad, 1e-6);
  EXPECT_GT(layout.buttons[1].x, layout.buttons[0].x + layout.buttons[0].w);
}

TEST(Toolbar, ButtonsSizeThemselvesFromTheirLabels) {
  RecordingRenderer r;
  const ToolbarLayout layout = layoutToolbar(headerLike(), kWidth, kCtx.layout, r);

  EXPECT_GT(layout.buttons[1].w, layout.buttons[0].w) << "\"Save As\" is a longer label than \"Save\"";
  EXPECT_GT(layout.buttons[0].w, r.measureText("Save").x);
}

TEST(Toolbar, ButtonAtFindsTheButtonUnderAPoint) {
  RecordingRenderer r;
  const ToolbarLayout layout = layoutToolbar(headerLike(), kWidth, kCtx.layout, r);

  EXPECT_EQ(buttonAt(layout, layout.buttons[3].center()), 3u);
  EXPECT_EQ(buttonAt(layout, layout.buttons[0].center()), 0u);
  EXPECT_EQ(buttonAt(layout, Vec2{kWidth / 2, 500}), std::nullopt);
}

TEST(Toolbar, AddingAButtonShiftsTheOthersButNotTheRightAligned) {
  RecordingRenderer r;
  Toolbar grown = headerLike();
  grown.buttons.insert(grown.buttons.begin(), Button{.label = "New", .onClick = [] {}});

  const ToolbarLayout before = layoutToolbar(headerLike(), kWidth, kCtx.layout, r);
  const ToolbarLayout after = layoutToolbar(grown, kWidth, kCtx.layout, r);

  EXPECT_GT(after.buttons[2].x, before.buttons[1].x) << "the new button pushes the rest right";
  EXPECT_EQ(after.buttons[4], before.buttons[3]) << "right-aligned buttons stay flush to the edge";
}

TEST(Button, DrawFillsBordersAndLabelsItsRect) {
  RecordingRenderer r;
  const Rect rect{0, 0, 25, 25};

  drawButton(r, Button{.label = "Run", .onClick = [] {}}, rect, kCtx);

  EXPECT_TRUE(hasFill(r.calls, rect));
  EXPECT_TRUE(hasRect(r.calls, rect));
  EXPECT_TRUE(hasTextAt(r.calls, "Run", Vec2{kCtx.layout.textPad, kCtx.layout.textPad}));
}
