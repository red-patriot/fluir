#include "editor/components/menu.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/renderer.hpp"
#include "recording_renderer.hpp"

// A menu drops rows under its anchor, sized to its longest label. Contracts are placement, hits and labels drawn.

namespace {

  using fluir::editor::drawMenu;
  using fluir::editor::EditorContext;
  using fluir::editor::GLYPH_PX;
  using fluir::editor::layoutMenu;
  using fluir::editor::menuItemAt;
  using fluir::editor::MenuLayout;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using testutil::RecordingRenderer;

  const EditorContext kCtx;
  const std::vector<std::string> kLabels{"+", "-", "&&", "=="};
  const Rect kAnchor{100, 100, 25, 25};
  const Rect kBounds{0, 0, 800, 600};

}  // namespace

TEST(Menu, RowsStackBelowTheAnchorOnePerLabel) {
  const MenuLayout uut = layoutMenu(kLabels, kAnchor, kBounds, kCtx.layout);

  ASSERT_EQ(uut.items.size(), kLabels.size());
  EXPECT_DOUBLE_EQ(uut.frame.x, kAnchor.x);
  EXPECT_DOUBLE_EQ(uut.frame.y, kAnchor.y + kAnchor.h);
  EXPECT_DOUBLE_EQ(uut.items.front().y, uut.frame.y);
  for (std::size_t i = 0; i < uut.items.size(); ++i) {
    EXPECT_GT(uut.items[i].h, 0);
    EXPECT_DOUBLE_EQ(uut.items[i].x, uut.frame.x);
    EXPECT_DOUBLE_EQ(uut.items[i].w, uut.frame.w);
    if (i > 0) {
      EXPECT_DOUBLE_EQ(uut.items[i].y, uut.items[i - 1].y + uut.items[i - 1].h);
    }
  }
  EXPECT_DOUBLE_EQ(uut.frame.y + uut.frame.h, uut.items.back().y + uut.items.back().h);
}

TEST(Menu, TheFrameIsAtLeastTheAnchorWidth) {
  const MenuLayout uut = layoutMenu(kLabels, Rect{100, 100, 300, 25}, kBounds, kCtx.layout);

  EXPECT_DOUBLE_EQ(uut.frame.w, 300);
}

TEST(Menu, TheFrameFitsTheLongestLabel) {
  const std::vector<std::string> labels{"+", "a much longer label"};
  const MenuLayout uut = layoutMenu(labels, kAnchor, kBounds, kCtx.layout);

  EXPECT_GE(uut.frame.w, static_cast<double>(labels[1].size()) * GLYPH_PX + 2 * kCtx.layout.textPad);
}

TEST(Menu, AMenuThatWouldOverflowTheBottomFlipsAboveTheAnchor) {
  const Rect low{100, 580, 25, 15};
  const MenuLayout uut = layoutMenu(kLabels, low, kBounds, kCtx.layout);

  EXPECT_DOUBLE_EQ(uut.frame.y + uut.frame.h, low.y);
  EXPECT_DOUBLE_EQ(uut.items.front().y, uut.frame.y);
}

TEST(Menu, AMenuPastTheRightEdgeIsClampedIntoBounds) {
  const MenuLayout uut = layoutMenu(kLabels, Rect{790, 100, 25, 25}, kBounds, kCtx.layout);

  EXPECT_DOUBLE_EQ(uut.frame.x + uut.frame.w, kBounds.w);
  EXPECT_DOUBLE_EQ(uut.items.front().x, uut.frame.x);
}

TEST(Menu, MenuItemAtFindsTheRowUnderAPoint) {
  const MenuLayout uut = layoutMenu(kLabels, kAnchor, kBounds, kCtx.layout);

  EXPECT_EQ(menuItemAt(uut, uut.items[0].center()), 0u);
  EXPECT_EQ(menuItemAt(uut, uut.items[2].center()), 2u);
  EXPECT_EQ(menuItemAt(uut, kAnchor.center()), std::nullopt);
  EXPECT_EQ(menuItemAt(uut, Vec2{700, 500}), std::nullopt);
}

TEST(Menu, DrawShowsEveryLabelAndHighlightsTheHoveredRow) {
  RecordingRenderer r;
  const MenuLayout uut = layoutMenu(kLabels, kAnchor, kBounds, kCtx.layout);

  drawMenu(r, kLabels, uut, 1u, kCtx);

  const std::vector<std::string> texts = testutil::textStrings(r.calls);
  for (const std::string& label : kLabels) {
    EXPECT_NE(std::ranges::find(texts, label), texts.end()) << label;
  }
  EXPECT_TRUE(testutil::hasFill(r.calls, uut.frame));
  EXPECT_TRUE(testutil::hasRect(r.calls, uut.frame));
  EXPECT_TRUE(testutil::hasFill(r.calls, uut.items[1]));
  EXPECT_FALSE(testutil::hasFill(r.calls, uut.items[2])) << "only the hovered row is highlighted";
}
