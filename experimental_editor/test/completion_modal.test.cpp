#include "editor/tools/completion_modal.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/tools/tool.hpp"
#include "recording_renderer.hpp"
#include "tool_harness.hpp"

// A centered half-width modal, as tall as its boxed rows; a press outside or Escape closes it.

namespace {

  using fluir::editor::CommentOption;
  using fluir::editor::Completion;
  using fluir::editor::CompletionModal;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::FunctionDefOption;
  using fluir::editor::InputEvent;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::key;
  using testutil::move;

  const EditorContext kCtx;
  const Rect kBounds{0, 0, 800, 600};

  // Labels are views, so they must outlive the modal.
  std::vector<Completion> completions(std::size_t n) {
    static constexpr std::array<std::string_view, 3> kLabels{"One", "Two", "Three"};
    std::vector<Completion> out;
    for (std::size_t i = 0; i < n; ++i) {
      out.push_back({kLabels.at(i), CommentOption{}});
    }
    return out;
  }

  struct Fixture {
    EditorState state{kCtx};
    CompletionModal uut{{{"Function", FunctionDefOption{}}, {"Comment", CommentOption{}}}, kBounds, nullptr};
  };

  // Reports text much taller than GLYPH_PX, like a real font's line height.
  struct TallTextRenderer : testutil::RecordingRenderer {
    static constexpr double kLinePx = 30.0;
    Vec2 measureText(std::string_view t) override { return {static_cast<double>(t.size()) * 8.0, kLinePx}; }
  };

  // Outlined rects other than the frame, top to bottom.
  std::vector<Rect> drawnRows(const CompletionModal& uut) {
    testutil::RecordingRenderer r;
    uut.draw(r, kCtx);
    std::vector<Rect> rows;
    for (const Rect& rect : testutil::rectsOf(r.calls)) {
      if (!(rect == uut.frame())) {
        rows.push_back(rect);
      }
    }
    std::ranges::sort(rows, {}, &Rect::y);
    return rows;
  }

}  // namespace

TEST(CompletionModal, TheFrameIsHalfTheBoundsWideAndCentered) {
  Fixture f;

  EXPECT_NEAR(f.uut.frame().w, 400, 1e-6);
  testutil::expectVecNear(f.uut.frame().center(), Vec2{400, 300});
}

TEST(CompletionModal, TheFrameGrowsWithTheNumberOfCompletions) {
  const CompletionModal one{completions(1), kBounds, nullptr};
  const CompletionModal two{completions(2), kBounds, nullptr};
  const CompletionModal three{completions(3), kBounds, nullptr};

  EXPECT_LT(one.frame().h, two.frame().h);
  EXPECT_LT(two.frame().h, three.frame().h);
  EXPECT_LT(three.frame().h, kBounds.h);
}

TEST(CompletionModal, EachRowIsItsOwnBoxWithGapsInsideTheFrame) {
  const CompletionModal uut{completions(3), kBounds, nullptr};
  const Rect frame = uut.frame();

  const auto rows = drawnRows(uut);

  ASSERT_EQ(rows.size(), 3u);
  EXPECT_GT(rows.front().y, frame.y) << "gap above the first row";
  EXPECT_LT(rows.back().y + rows.back().h, frame.y + frame.h) << "gap below the last row";
  for (std::size_t i = 0; i < rows.size(); ++i) {
    EXPECT_GT(rows[i].x, frame.x);
    EXPECT_LT(rows[i].x + rows[i].w, frame.x + frame.w);
    if (i > 0) {
      EXPECT_GT(rows[i].y, rows[i - 1].y + rows[i - 1].h) << "gap between rows " << i - 1 << " and " << i;
    }
  }
}

TEST(CompletionModal, APressOutsideClosesWithAnyButton) {
  Fixture f;

  EXPECT_FALSE(f.uut.onEvent(down(Vec2{100, 100}), f.state));
  EXPECT_FALSE(f.uut.onEvent(down(Vec2{700, 500}, InputEvent::Button::Right), f.state));
  EXPECT_FALSE(f.uut.onEvent(down(Vec2{400, 500}, InputEvent::Button::Middle), f.state));
}

TEST(CompletionModal, EscapeCloses) {
  Fixture f;

  EXPECT_FALSE(f.uut.onEvent(key(InputEvent::Key::Escape), f.state));
}

TEST(CompletionModal, PressesInsideMovesAndOtherKeysKeepItOpen) {
  Fixture f;

  const auto rows = drawnRows(f.uut);
  ASSERT_EQ(rows.size(), 2u);
  const double gapY = (rows[0].y + rows[0].h + rows[1].y) / 2;

  EXPECT_TRUE(f.uut.onEvent(down(rows[0].center()), f.state)) << "on the first row";
  EXPECT_TRUE(f.uut.onEvent(down(Vec2{rows[0].x + 10, gapY}), f.state)) << "in the gap between rows";
  EXPECT_TRUE(f.uut.onEvent(move(Vec2{10, 10}), f.state));
  EXPECT_TRUE(f.uut.onEvent(testutil::up(Vec2{10, 10}), f.state));
  EXPECT_TRUE(f.uut.onEvent(key(InputEvent::Key::Delete), f.state));
  EXPECT_TRUE(f.uut.onEvent(testutil::text("x"), f.state));
}

TEST(CompletionModal, DrawShowsEachLabelEnlargedInsideItsRow) {
  Fixture f;
  testutil::RecordingRenderer r;
  const auto rows = drawnRows(f.uut);
  ASSERT_EQ(rows.size(), 2u);

  f.uut.draw(r, kCtx);

  EXPECT_TRUE(testutil::hasFill(r.calls, f.uut.frame()));
  const auto texts = testutil::opsOf(r.calls, testutil::DrawCall::Op::Text);
  ASSERT_EQ(testutil::textStrings(r.calls), (std::vector<std::string>{"Function", "Comment"}));
  for (std::size_t i = 0; i < texts.size(); ++i) {
    EXPECT_TRUE(rows[i].contains(texts[i].a)) << texts[i].text;
    EXPECT_NEAR(texts[i].scale, 1.25, 1e-6) << texts[i].text;
  }
  EXPECT_EQ(f.uut.labels(), (std::vector<std::string>{"Function", "Comment"}));
}

TEST(CompletionModal, RowsGrowToFitTheMeasuredTextHeight) {
  TallTextRenderer tall;
  const CompletionModal uut{completions(2), kBounds, &tall};
  const CompletionModal fallback{completions(2), kBounds, nullptr};

  tall.calls.clear();
  uut.draw(tall, kCtx);

  EXPECT_GT(uut.frame().h, fallback.frame().h);
  const auto rows = drawnRows(uut);
  const auto texts = testutil::opsOf(tall.calls, testutil::DrawCall::Op::Text);
  ASSERT_EQ(rows.size(), 2u);
  ASSERT_EQ(texts.size(), 2u);
  for (std::size_t i = 0; i < rows.size(); ++i) {
    const double textBottom = texts[i].a.y + TallTextRenderer::kLinePx * texts[i].scale;
    EXPECT_GE(texts[i].a.y, rows[i].y) << texts[i].text;
    EXPECT_LE(textBottom, rows[i].y + rows[i].h) << texts[i].text << " overflows its row";
  }
}
