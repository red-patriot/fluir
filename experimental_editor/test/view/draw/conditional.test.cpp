#include "editor/view/draw/conditional.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/core/viewport.hpp"
#include "recording_renderer.hpp"

// A conditional paints in parts at different depths, so each part is driven on its own here.
// Contracts only: which primitive lands where, never how many.

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::ELSE_BRANCH_ID;
  using fluir::editor::Rect;
  using fluir::editor::Subview;
  using fluir::editor::THEN_BRANCH_ID;
  using fluir::editor::Viewport;
  using testutil::hasFill;
  using testutil::hasRect;
  using testutil::RecordingRenderer;
  using testutil::textStrings;

  const EditorContext kCtx;
  constexpr double kHeaderH = 25;  // headerUnits 5 * unitPx 5

  fluir::pt::Conditional makeConditional() {
    return fluir::pt::Conditional{.id = 20,
                                  .location = {.x = 0, .y = 0, .z = 0, .width = 20, .height = 18},
                                  .condition = {},
                                  .inputs = {},
                                  .outputs = {},
                                  .thenScope = xyz::indirect<fluir::pt::Block>{},
                                  .elseScope = xyz::indirect<fluir::pt::Block>{}};
  }

  // An identity view: world px are screen px, so the rects asserted below are the ones passed in.
  Subview rootView(RecordingRenderer& r, const Viewport& viewport) {
    return Subview{viewport, Rect{0, 0, r.outputSize().x, r.outputSize().y}, r};
  }

  bool hasText(const RecordingRenderer& r, std::string_view text) {
    const std::vector<std::string> texts = textStrings(r.calls);
    return std::find(texts.begin(), texts.end(), std::string{text}) != texts.end();
  }

}  // namespace

TEST(DrawConditional, ColorIsTheConditionalHeaderTheme) {
  EXPECT_EQ(fluir::editor::draw::color(makeConditional(), kCtx.theme), kCtx.theme.conditionalNodeHeader);
}

// Wiring through block terminals is Phase 2; a conditional has no wall terminals to anchor yet.
TEST(DrawConditional, AnchorsAreEmpty) {
  const fluir::editor::TerminalSet terminals =
    fluir::editor::draw::anchors(makeConditional(), Rect{10, 35, 100, 90}, kCtx.layout);

  EXPECT_TRUE(terminals.inputs.empty());
  EXPECT_TRUE(terminals.outputs.empty());
}

TEST(DrawConditional, BodyFillsItsWholeFrame) {
  RecordingRenderer r;
  const Viewport viewport;

  fluir::editor::draw::drawBody(makeConditional(), Rect{10, 35, 100, 90}, rootView(r, viewport), kCtx);

  EXPECT_TRUE(hasFill(r.calls, Rect{10, 35, 100, 90}));
}

TEST(DrawConditional, BranchTagNamesTheBranchIndex) {
  EXPECT_EQ(fluir::editor::draw::branchTag(THEN_BRANCH_ID), fluir::editor::draw::THEN_TAG);
  EXPECT_EQ(fluir::editor::draw::branchTag(ELSE_BRANCH_ID), fluir::editor::draw::ELSE_TAG);
}

// One frame, one header band: the frame labels the branch it shows.
TEST(DrawConditional, FrameFillsItsHeaderBandAndTagsIt) {
  RecordingRenderer r;
  const Viewport viewport;

  fluir::editor::draw::drawFrame(makeConditional(), Rect{10, 35, 100, 90}, rootView(r, viewport), kCtx);

  EXPECT_TRUE(hasFill(r.calls, Rect{10, 35, 100, kHeaderH}));
  EXPECT_TRUE(hasRect(r.calls, Rect{10, 35, 100, 90}));  // the border spans the whole frame
  EXPECT_TRUE(hasText(r, fluir::editor::draw::THEN_TAG));
}
