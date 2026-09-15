#include "editor/components/menu.hpp"

#include <algorithm>
#include <string>
#include <string_view>
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
  const MenuLayout uut = layoutMenu(kLabels, kAnchor, kBounds, kCtx.layout, nullptr);

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
  const MenuLayout uut = layoutMenu(kLabels, Rect{100, 100, 300, 25}, kBounds, kCtx.layout, nullptr);

  EXPECT_DOUBLE_EQ(uut.frame.w, 300);
}

TEST(Menu, TheFrameFitsTheLongestLabel) {
  const std::vector<std::string> labels{"+", "a much longer label"};
  const MenuLayout uut = layoutMenu(labels, kAnchor, kBounds, kCtx.layout, nullptr);

  EXPECT_GE(uut.frame.w, static_cast<double>(labels[1].size()) * GLYPH_PX + 2 * kCtx.layout.textPad);
}

TEST(Menu, AMenuThatWouldOverflowTheBottomFlipsAboveTheAnchor) {
  const Rect low{100, 580, 25, 15};
  const MenuLayout uut = layoutMenu(kLabels, low, kBounds, kCtx.layout, nullptr);

  EXPECT_DOUBLE_EQ(uut.frame.y + uut.frame.h, low.y);
  EXPECT_DOUBLE_EQ(uut.items.front().y, uut.frame.y);
}

TEST(Menu, AMenuPastTheRightEdgeIsClampedIntoBounds) {
  const MenuLayout uut = layoutMenu(kLabels, Rect{790, 100, 25, 25}, kBounds, kCtx.layout, nullptr);

  EXPECT_DOUBLE_EQ(uut.frame.x + uut.frame.w, kBounds.w);
  EXPECT_DOUBLE_EQ(uut.items.front().x, uut.frame.x);
}

TEST(Menu, MenuItemAtFindsTheRowUnderAPoint) {
  const MenuLayout uut = layoutMenu(kLabels, kAnchor, kBounds, kCtx.layout, nullptr);

  EXPECT_EQ(menuItemAt(uut, uut.items[0].center()), 0u);
  EXPECT_EQ(menuItemAt(uut, uut.items[2].center()), 2u);
  EXPECT_EQ(menuItemAt(uut, kAnchor.center()), std::nullopt);
  EXPECT_EQ(menuItemAt(uut, Vec2{700, 500}), std::nullopt);
}

TEST(Menu, DrawShowsEveryLabelAndHighlightsTheHoveredRow) {
  RecordingRenderer r;
  const MenuLayout uut = layoutMenu(kLabels, kAnchor, kBounds, kCtx.layout, nullptr);

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

namespace {

  // Real fonts run wider and taller than the GLYPH_PX estimate.
  class WideFont : public RecordingRenderer {
   public:
    Vec2 measureText(std::string_view t) override { return {static_cast<double>(t.size()) * 13.0, 30.0}; }
  };

}  // namespace

TEST(Menu, TheFrameAndRowsFitMeasuredLabels) {
  WideFont font;
  const std::vector<std::string> labels{"+", "a much longer label"};
  const MenuLayout uut = layoutMenu(labels, kAnchor, kBounds, kCtx.layout, &font);

  EXPECT_GE(uut.frame.w, font.measureText(labels[1]).x + 2 * kCtx.layout.textPad);
  for (const Rect& row : uut.items) {
    EXPECT_GE(row.h, font.measureText(labels[1]).y + 2 * kCtx.layout.textPad);
  }
}

TEST(Menu, DrawnLabelsLieInsideTheirRows) {
  WideFont font;
  const std::vector<std::string> labels{"+", "a much longer label"};
  const MenuLayout uut = layoutMenu(labels, kAnchor, kBounds, kCtx.layout, &font);

  drawMenu(font, labels, uut, std::nullopt, kCtx);

  for (std::size_t i = 0; i < labels.size(); ++i) {
    const auto texts = testutil::opsOf(font.calls, testutil::DrawCall::Op::Text);
    const auto it = std::ranges::find(texts, labels[i], &testutil::DrawCall::text);
    ASSERT_NE(it, texts.end()) << labels[i];
    const Vec2 size = font.measureText(labels[i]);
    const Rect& row = uut.items[i];
    EXPECT_GE(it->a.x, row.x);
    EXPECT_GE(it->a.y, row.y);
    EXPECT_LE(it->a.x + size.x, row.x + row.w);
    EXPECT_LE(it->a.y + size.y, row.y + row.h);
    EXPECT_LE(row.y + row.h, uut.frame.y + uut.frame.h);
  }
}

TEST(Menu, AMenuTallerThanTheSpaceAboveAndBelowStaysInsideTheTop) {
  const std::vector<std::string> labels(10, "row");
  const Rect bounds{0, 0, 800, 100};
  const MenuLayout uut = layoutMenu(labels, Rect{100, 50, 25, 10}, bounds, kCtx.layout, nullptr);

  EXPECT_DOUBLE_EQ(uut.frame.y, bounds.y);
  EXPECT_DOUBLE_EQ(uut.items.front().y, uut.frame.y);
}
