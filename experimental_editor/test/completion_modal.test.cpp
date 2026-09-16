#include "editor/tools/completion_modal.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
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

  InputEvent wheel(double y) { return {.type = InputEvent::Type::Wheel, .pos = {400, 300}, .wheel = {0, y}}; }

  const EditorContext kCtx;
  const Rect kBounds{0, 0, 800, 600};
  constexpr fluir::Coordinate kWhere{.x = 12, .y = 34, .z = 0};

  // Labels are views, so they must outlive the modal.
  std::vector<Completion> completions(std::size_t n) {
    static const std::vector<std::string> kLabels = [] {
      std::vector<std::string> labels;
      for (int i = 0; i < 40; ++i) {
        labels.push_back("Item " + std::to_string(i));
      }
      return labels;
    }();
    std::vector<Completion> out;
    for (std::size_t i = 0; i < n; ++i) {
      out.push_back({kLabels.at(i), CommentOption{}});
    }
    return out;
  }

  struct Fixture {
    EditorState state{kCtx};
    CompletionModal uut{
      {{"Function", FunctionDefOption{}}, {"Comment", CommentOption{}}}, kBounds, nullptr, kWhere, fluir::FullID{}};
  };

  // Reports text much taller than GLYPH_PX, like a real font's line height.
  struct TallTextRenderer : testutil::RecordingRenderer {
    static constexpr double kLinePx = 30.0;
    Vec2 measureText(std::string_view t) override { return {static_cast<double>(t.size()) * 8.0, kLinePx}; }
  };

  // Outlined rects other than the frame and the search bar (drawn first), top to bottom.
  std::vector<Rect> drawnRows(const CompletionModal& uut) {
    testutil::RecordingRenderer r;
    uut.draw(r, kCtx);
    const std::vector<Rect> rects = testutil::rectsOf(r.calls);
    std::vector<Rect> rows;
    for (std::size_t i = 1; i < rects.size(); ++i) {
      if (!(rects[i] == uut.frame())) {
        rows.push_back(rects[i]);
      }
    }
    std::ranges::sort(rows, {}, &Rect::y);
    return rows;
  }

  // Row labels drawn, excluding the search bar's query text.
  std::vector<std::string> drawnLabels(const CompletionModal& uut) {
    testutil::RecordingRenderer r;
    uut.draw(r, kCtx);
    std::vector<std::string> out;
    for (const auto& call : testutil::opsOf(r.calls, testutil::DrawCall::Op::Text)) {
      if (!uut.searchBar().contains(call.a)) {
        out.push_back(call.text);
      }
    }
    return out;
  }

  // Wheels `uut` until row `label` is fully inside the frame, then returns its drawn rect.
  std::optional<Rect> scrollTo(CompletionModal& uut, EditorState& state, std::string_view label) {
    const auto it = std::ranges::find(uut.labels(), label);
    if (it == uut.labels().end()) {
      return std::nullopt;
    }
    const auto index = static_cast<std::size_t>(it - uut.labels().begin());
    for (int step = 0; step < 100; ++step) {
      const Rect row = drawnRows(uut).at(index);
      const Rect frame = uut.frame();
      if (row.y >= frame.y && row.y + row.h <= frame.y + frame.h) {
        return row;
      }
      uut.onEvent(wheel(row.y < frame.y ? 1 : -1), state);
    }
    return std::nullopt;
  }

  // simple_binary_expr.fl: function 1 (z 3) holds binary 1 and constants 2, 3.
  struct BodyFixture {
    EditorState state{kCtx};
    std::optional<CompletionModal> uut;

    BodyFixture() {
      testutil::loadInto(state, "read/simple_binary_expr.fl");
      const fluir::Coordinate where{.x = kWhere.x, .y = kWhere.y, .z = 3};
      uut.emplace(state.intelligence.completions(state.editor.tree(), fluir::FullID{1}),
                  kBounds,
                  nullptr,
                  where,
                  fluir::FullID{1});
    }

    const fluir::pt::Block& body() const {
      return std::get<fluir::pt::FunctionDecl>(state.editor.tree().declarations.at(1)).body;
    }

    // Picks `label` and returns the single node it added.
    const fluir::pt::Node* pick(std::string_view label) {
      const auto row = scrollTo(*uut, state, label);
      if (!row) {
        return nullptr;
      }
      const auto before = body().nodes;
      if (uut->onEvent(down(row->center()), state)) {
        return nullptr;
      }
      for (const auto& [id, node] : body().nodes) {
        if (!before.contains(id)) {
          return &node;
        }
      }
      return nullptr;
    }
  };

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

TEST(CompletionModal, NonPickingPressesInsideMovesAndOtherKeysKeepItOpen) {
  Fixture f;

  const auto rows = drawnRows(f.uut);
  ASSERT_EQ(rows.size(), 2u);
  const double gapY = (rows[0].y + rows[0].h + rows[1].y) / 2;

  EXPECT_TRUE(f.uut.onEvent(down(rows[0].center(), InputEvent::Button::Right), f.state)) << "right on the first row";
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

TEST(CompletionModal, ALeftPressOnFunctionAddsAFunctionAtThePointAndCloses) {
  Fixture f;
  const auto rows = drawnRows(f.uut);
  ASSERT_EQ(rows.size(), 2u);

  EXPECT_FALSE(f.uut.onEvent(down(rows[0].center()), f.state));

  const auto& decls = f.state.editor.tree().declarations;
  ASSERT_EQ(decls.size(), 1u);
  const auto* fn = std::get_if<fluir::pt::FunctionDecl>(&decls.begin()->second);
  ASSERT_NE(fn, nullptr);
  EXPECT_EQ(fn->location.x, kWhere.x);
  EXPECT_EQ(fn->location.y, kWhere.y);
  EXPECT_TRUE(f.state.editor.undo());
  EXPECT_TRUE(f.state.editor.tree().declarations.empty());
}

TEST(CompletionModal, ALeftPressOnCommentAddsACommentAtThePointAndCloses) {
  Fixture f;
  const auto rows = drawnRows(f.uut);
  ASSERT_EQ(rows.size(), 2u);

  EXPECT_FALSE(f.uut.onEvent(down(rows[1].center()), f.state));

  const auto& decls = f.state.editor.tree().declarations;
  ASSERT_EQ(decls.size(), 1u);
  const auto* comment = std::get_if<fluir::pt::Comment>(&decls.begin()->second);
  ASSERT_NE(comment, nullptr);
  EXPECT_EQ(comment->location.x, kWhere.x);
  EXPECT_EQ(comment->location.y, kWhere.y);
  EXPECT_TRUE(f.state.editor.undo());
  EXPECT_TRUE(f.state.editor.tree().declarations.empty());
}

TEST(CompletionModal, OtherButtonsOnARowAndPressesInAGapAddNothing) {
  Fixture f;
  const auto rows = drawnRows(f.uut);
  ASSERT_EQ(rows.size(), 2u);
  const double gapY = (rows[0].y + rows[0].h + rows[1].y) / 2;

  EXPECT_TRUE(f.uut.onEvent(down(rows[0].center(), InputEvent::Button::Right), f.state));
  EXPECT_TRUE(f.uut.onEvent(down(rows[1].center(), InputEvent::Button::Middle), f.state));
  EXPECT_TRUE(f.uut.onEvent(down(Vec2{rows[0].x + 10, gapY}), f.state));

  EXPECT_TRUE(f.state.editor.tree().declarations.empty());
  EXPECT_FALSE(f.state.editor.canUndo());
}

TEST(CompletionModal, HoveringARowFillsIt) {
  Fixture f;
  const auto rows = drawnRows(f.uut);
  ASSERT_EQ(rows.size(), 2u);

  testutil::RecordingRenderer idle;
  f.uut.draw(idle, kCtx);
  EXPECT_FALSE(testutil::hasFill(idle.calls, rows[0]));
  EXPECT_FALSE(testutil::hasFill(idle.calls, rows[1]));

  EXPECT_TRUE(f.uut.onEvent(move(rows[1].center()), f.state));
  testutil::RecordingRenderer hovered;
  f.uut.draw(hovered, kCtx);
  EXPECT_FALSE(testutil::hasFill(hovered.calls, rows[0]));
  EXPECT_TRUE(testutil::hasFill(hovered.calls, rows[1]));

  EXPECT_TRUE(f.uut.onEvent(move(Vec2{1, 1}), f.state));
  testutil::RecordingRenderer left;
  f.uut.draw(left, kCtx);
  EXPECT_FALSE(testutil::hasFill(left.calls, rows[1]));
}

TEST(CompletionModal, ATallListIsCappedAtTheBoundsAndClipsItsRows) {
  const CompletionModal uut{completions(40), kBounds, nullptr};
  testutil::RecordingRenderer r;

  uut.draw(r, kCtx);

  EXPECT_LE(uut.frame().h, kBounds.h);
  EXPECT_GE(uut.frame().y, kBounds.y);
  EXPECT_EQ(testutil::countOf(r.calls, testutil::DrawCall::Op::PushClip),
            testutil::countOf(r.calls, testutil::DrawCall::Op::PopClip));
  const auto rows = drawnRows(uut);
  ASSERT_EQ(rows.size(), 40u);
  EXPECT_GT(rows.back().y, uut.frame().y + uut.frame().h) << "last row starts clipped below the frame";
}

TEST(CompletionModal, WheelingDownScrollsAClippedRowIntoReach) {
  EditorState state{kCtx};
  CompletionModal uut{completions(40), kBounds, nullptr, kWhere};
  const auto before = drawnRows(uut);
  ASSERT_EQ(before.size(), 40u);

  EXPECT_TRUE(uut.onEvent(move(before.back().center()), state));
  testutil::RecordingRenderer idle;
  uut.draw(idle, kCtx);
  for (const Rect& row : before) {
    EXPECT_FALSE(testutil::hasFill(idle.calls, row)) << "no row fills while the pointer is outside the frame";
  }

  EXPECT_TRUE(uut.onEvent(wheel(-1000), state));
  const auto after = drawnRows(uut);
  ASSERT_EQ(after.size(), 40u);
  EXPECT_TRUE(uut.frame().contains(after.back().center()));
  EXPECT_FALSE(uut.frame().contains(after.front().center())) << "first row scrolled out";

  EXPECT_TRUE(uut.onEvent(move(after.back().center()), state));
  testutil::RecordingRenderer hovered;
  uut.draw(hovered, kCtx);
  EXPECT_TRUE(testutil::hasFill(hovered.calls, after.back()));

  EXPECT_TRUE(uut.onEvent(move(before.front().center()), state));
  testutil::RecordingRenderer overFirst;
  uut.draw(overFirst, kCtx);
  EXPECT_FALSE(testutil::hasFill(overFirst.calls, after.front())) << "first row no longer under its old spot";

  EXPECT_FALSE(uut.onEvent(down(after.back().center()), state));
  EXPECT_EQ(state.editor.tree().declarations.size(), 1u);
}

TEST(CompletionModal, TheWheelClampsAtBothEnds) {
  EditorState state{kCtx};
  CompletionModal uut{completions(40), kBounds, nullptr};
  const auto top = drawnRows(uut);

  EXPECT_TRUE(uut.onEvent(wheel(-1000), state));
  const auto bottom = drawnRows(uut);
  EXPECT_NEAR(bottom.back().y + bottom.back().h, uut.frame().y + uut.frame().h - 4.0, 1e-6)
    << "last row rests on the bottom gap";
  EXPECT_TRUE(uut.onEvent(wheel(-5), state));
  EXPECT_EQ(drawnRows(uut), bottom);

  EXPECT_TRUE(uut.onEvent(wheel(1000), state));
  EXPECT_EQ(drawnRows(uut), top);
  EXPECT_TRUE(uut.onEvent(wheel(5), state));
  EXPECT_EQ(drawnRows(uut), top);
}

TEST(CompletionModal, AShortListIgnoresTheWheel) {
  EditorState state{kCtx};
  CompletionModal uut{completions(2), kBounds, nullptr};
  const auto rows = drawnRows(uut);

  EXPECT_TRUE(uut.onEvent(wheel(-3), state));
  EXPECT_EQ(drawnRows(uut), rows);
  EXPECT_TRUE(uut.onEvent(wheel(3), state));
  EXPECT_EQ(drawnRows(uut), rows);

  EXPECT_FALSE(uut.onEvent(down(rows[0].center()), state));
  EXPECT_EQ(state.editor.tree().declarations.size(), 1u);
}

TEST(CompletionModal, PickingABinaryOperatorAddsItInTheBodyAtThePoint) {
  BodyFixture f;

  const auto* node = f.pick("* (binary)");

  ASSERT_NE(node, nullptr);
  const auto* binary = std::get_if<fluir::pt::Binary>(node);
  ASSERT_NE(binary, nullptr);
  EXPECT_EQ(binary->op, fluir::Operator::STAR);
  EXPECT_EQ(binary->location, (fluir::FlowGraphLocation{.x = 12, .y = 34, .z = 4, .width = 8, .height = 5}));
  EXPECT_TRUE(f.state.editor.undo());
  EXPECT_EQ(f.body().nodes.size(), 3u);
}

TEST(CompletionModal, PickingAUnaryOperatorAddsItInTheBody) {
  BodyFixture f;

  const auto* node = f.pick("! (unary)");

  ASSERT_NE(node, nullptr);
  const auto* unary = std::get_if<fluir::pt::Unary>(node);
  ASSERT_NE(unary, nullptr);
  EXPECT_EQ(unary->op, fluir::Operator::BANG);
  EXPECT_EQ(unary->location.x, kWhere.x);
  EXPECT_EQ(unary->location.y, kWhere.y);
}

TEST(CompletionModal, PickingAConstantAddsADefaultValueSizedForItsType) {
  BodyFixture f;

  const auto* i32 = f.pick("I32");
  ASSERT_NE(i32, nullptr);
  const auto* constant = std::get_if<fluir::pt::Constant>(i32);
  ASSERT_NE(constant, nullptr);
  EXPECT_EQ(constant->value, (fluir::literals_types::Literal{fluir::literals_types::I32{0}}));
  EXPECT_EQ(constant->location, (fluir::FlowGraphLocation{.x = 12, .y = 34, .z = 4, .width = 12, .height = 5}));
  EXPECT_TRUE(f.state.editor.undo());
  EXPECT_EQ(f.body().nodes.size(), 3u);
}

TEST(CompletionModal, ABoolConstantIsNarrower) {
  BodyFixture f;

  const auto* node = f.pick("BOOL");

  ASSERT_NE(node, nullptr);
  const auto* constant = std::get_if<fluir::pt::Constant>(node);
  ASSERT_NE(constant, nullptr);
  EXPECT_EQ(constant->value, (fluir::literals_types::Literal{false}));
  EXPECT_EQ(constant->location.width, 8);
  EXPECT_EQ(constant->location.height, 5);
}

TEST(CompletionModal, PickingCommentInABodyAddsABodyComment) {
  BodyFixture f;

  const auto* node = f.pick("Comment");

  ASSERT_NE(node, nullptr);
  EXPECT_TRUE(std::holds_alternative<fluir::pt::Comment>(*node));
}

TEST(CompletionModal, ALongListShowsAtMostTenRows) {
  const CompletionModal ten{completions(10), kBounds, nullptr};
  const CompletionModal eleven{completions(11), kBounds, nullptr};
  const CompletionModal forty{completions(40), kBounds, nullptr};
  const Rect tallBounds{0, 0, 800, 4000};
  const CompletionModal fortyInTallBounds{completions(40), tallBounds, nullptr};

  EXPECT_NEAR(eleven.frame().h, ten.frame().h, 1e-6);
  EXPECT_NEAR(forty.frame().h, ten.frame().h, 1e-6);
  EXPECT_NEAR(fortyInTallBounds.frame().h, ten.frame().h, 1e-6) << "not as many as the bounds fit";
}

TEST(CompletionModal, ASearchBarSitsAboveTheRowsInsideTheFrame) {
  Fixture f;
  const Rect frame = f.uut.frame();
  const Rect bar = f.uut.searchBar();

  const auto rows = drawnRows(f.uut);

  ASSERT_EQ(rows.size(), 2u);
  EXPECT_GT(bar.y, frame.y);
  EXPECT_GT(bar.x, frame.x);
  EXPECT_LT(bar.x + bar.w, frame.x + frame.w);
  EXPECT_NEAR(bar.h, rows[0].h, 1e-6);
  for (const Rect& row : rows) {
    EXPECT_GE(row.y, bar.y + bar.h) << "rows sit below the bar";
  }
}

TEST(CompletionModal, TypingDrawsTheQueryInTheSearchBar) {
  Fixture f;

  EXPECT_TRUE(f.uut.onEvent(testutil::text("bi"), f.state));

  testutil::RecordingRenderer r;
  f.uut.draw(r, kCtx);
  const auto texts = testutil::opsOf(r.calls, testutil::DrawCall::Op::Text);
  ASSERT_FALSE(texts.empty());
  EXPECT_EQ(texts.front().text, "bi");
  EXPECT_TRUE(f.uut.searchBar().contains(texts.front().a));
  const auto carets = testutil::fillsOfSize(r.calls, 1.0, f.uut.searchBar().h - 12.0);
  ASSERT_EQ(carets.size(), 1u);
  EXPECT_GT(carets[0].x, texts.front().a.x) << "the caret follows the query";
  EXPECT_TRUE(f.uut.searchBar().contains(carets[0].topLeft()));
}

TEST(CompletionModal, TypingFiltersRowsToLabelsThatContainIt) {
  BodyFixture f;
  std::vector<std::string> expected;
  for (const std::string& label : f.uut->labels()) {
    if (label.find("bin") != std::string::npos) {
      expected.push_back(label);
    }
  }
  ASSERT_FALSE(expected.empty());

  EXPECT_TRUE(f.uut->onEvent(testutil::text("bin"), f.state));

  EXPECT_EQ(drawnLabels(*f.uut), expected);
}

TEST(CompletionModal, TheFilterIgnoresCase) {
  BodyFixture f;

  EXPECT_TRUE(f.uut->onEvent(testutil::text("i32"), f.state));

  const auto labels = drawnLabels(*f.uut);
  EXPECT_NE(std::ranges::find(labels, "I32"), labels.end());
}

TEST(CompletionModal, BackspaceWidensTheFilter) {
  Fixture f;
  EXPECT_TRUE(f.uut.onEvent(testutil::text("c"), f.state));
  const auto wide = drawnLabels(f.uut);
  ASSERT_EQ(wide, (std::vector<std::string>{"Function", "Comment"}));
  EXPECT_TRUE(f.uut.onEvent(testutil::text("o"), f.state));
  EXPECT_EQ(drawnLabels(f.uut), (std::vector<std::string>{"Comment"}));

  EXPECT_TRUE(f.uut.onEvent(key(InputEvent::Key::Backspace), f.state));

  EXPECT_EQ(drawnLabels(f.uut), wide);
}

TEST(CompletionModal, AQueryThatMatchesNothingLeavesJustTheSearchBar) {
  Fixture f;

  EXPECT_TRUE(f.uut.onEvent(testutil::text("zzz"), f.state));

  EXPECT_TRUE(drawnRows(f.uut).empty());
  EXPECT_TRUE(f.uut.frame().contains(f.uut.searchBar().center()));
  EXPECT_TRUE(f.uut.onEvent(down(f.uut.frame().center()), f.state));
  EXPECT_TRUE(f.state.editor.tree().declarations.empty());
}

TEST(CompletionModal, TheFrameShrinksAsTheFilterNarrows) {
  Fixture f;
  const double wide = f.uut.frame().h;

  EXPECT_TRUE(f.uut.onEvent(testutil::text("Comm"), f.state));

  EXPECT_LT(f.uut.frame().h, wide);
  testutil::expectVecNear(f.uut.frame().center(), kBounds.center());
  EXPECT_TRUE(f.uut.onEvent(testutil::text("zzz"), f.state));
  EXPECT_LT(f.uut.frame().h, wide);
  testutil::expectVecNear(f.uut.frame().center(), kBounds.center());
}

TEST(CompletionModal, PickingAFilteredRowAddsThatOption) {
  BodyFixture f;
  EXPECT_TRUE(f.uut->onEvent(testutil::text("* (binary)"), f.state));
  const auto rows = drawnRows(*f.uut);
  ASSERT_EQ(rows.size(), 1u);
  const auto before = f.body().nodes;

  EXPECT_FALSE(f.uut->onEvent(down(rows[0].center()), f.state));

  const fluir::pt::Binary* binary = nullptr;
  for (const auto& [id, node] : f.body().nodes) {
    if (!before.contains(id)) {
      binary = std::get_if<fluir::pt::Binary>(&node);
    }
  }
  ASSERT_NE(binary, nullptr);
  EXPECT_EQ(binary->op, fluir::Operator::STAR);
}

TEST(CompletionModal, EnterKeepsTheModalOpenAndAddsNothing) {
  Fixture f;

  EXPECT_TRUE(f.uut.onEvent(key(InputEvent::Key::Return), f.state));

  EXPECT_TRUE(f.state.editor.tree().declarations.empty());
  EXPECT_EQ(drawnRows(f.uut).size(), 2u);
}

TEST(CompletionModal, FilteringResetsTheScroll) {
  EditorState state{kCtx};
  CompletionModal uut{completions(40), kBounds, nullptr};
  EXPECT_TRUE(uut.onEvent(wheel(-1000), state));

  EXPECT_TRUE(uut.onEvent(testutil::text("Item 1"), state));

  const auto rows = drawnRows(uut);
  ASSERT_EQ(rows.size(), 11u) << "Item 1 and Item 10..19";
  EXPECT_NEAR(rows.front().y, uut.searchBar().y + uut.searchBar().h + 4.0, 1e-6);
}
