#include "editor/view/draw/conditional.hpp"

#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/viewport.hpp"
#include "recording_renderer.hpp"

// A conditional paints in parts at different depths, so each part is driven on its own here.
// Contracts only: which primitive lands where, never how many.

namespace {

  using fluir::FlowGraphLocation;
  using fluir::ID;
  using fluir::editor::EditorContext;
  using fluir::editor::Rect;
  using fluir::editor::Subview;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::DrawCall;
  using testutil::hasFill;
  using testutil::hasLine;
  using testutil::hasRect;
  using testutil::RecordingRenderer;
  using testutil::textStrings;

  const EditorContext kCtx;
  constexpr double kHeaderH = 25;  // headerUnits 5 * unitPx 5

  fluir::pt::Scope makeScope(ID id, int y, int h) {
    return fluir::pt::Scope{.id = id, .location = {.x = 0, .y = y, .z = 0, .width = 20, .height = h}, .body = {}};
  }

  fluir::pt::Conditional makeConditional() {
    return fluir::pt::Conditional{.id = 20,
                                  .location = {.x = 0, .y = 0, .z = 0, .width = 20, .height = 18},
                                  .condition = {},
                                  .inputs = {},
                                  .outputs = {},
                                  .thenScope = xyz::indirect{makeScope(0, 0, 12)},
                                  .elseScope = xyz::indirect{makeScope(1, 12, 6)}};
  }

  // An identity view: world px are screen px, so the rects asserted below are the ones passed in.
  Subview rootView(RecordingRenderer& r, const Viewport& viewport) {
    return Subview{viewport, Rect{0, 0, r.outputSize().x, r.outputSize().y}, r};
  }

}  // namespace

TEST(DrawConditional, ColorIsTheConditionalHeaderTheme) {
  EXPECT_EQ(fluir::editor::draw::color(makeConditional(), kCtx.theme), kCtx.theme.conditionalNodeHeader);
}

// Wiring through scope ports is Phase 2; a conditional has no wall ports to anchor yet.
TEST(DrawConditional, AnchorsAreEmpty) {
  const fluir::editor::PortSet ports =
    fluir::editor::draw::anchors(makeConditional(), Rect{10, 35, 100, 90}, kCtx.layout);

  EXPECT_TRUE(ports.inputs.empty());
  EXPECT_TRUE(ports.outputs.empty());
}

TEST(DrawConditional, BodyFillsItsWholeFrame) {
  RecordingRenderer r;
  const Viewport viewport;

  fluir::editor::draw::drawBody(makeConditional(), Rect{10, 35, 100, 90}, rootView(r, viewport), kCtx);

  EXPECT_TRUE(hasFill(r.calls, Rect{10, 35, 100, 90}));
}

TEST(DrawConditional, ScopeDividesItselfFromWhatIsAboveIt) {
  RecordingRenderer r;
  const Viewport viewport;
  const fluir::pt::Conditional conditional = makeConditional();

  fluir::editor::draw::drawScope(
    *conditional.elseScope, fluir::editor::draw::ELSE_TAG, Rect{10, 95, 100, 30}, rootView(r, viewport), kCtx);

  EXPECT_TRUE(hasLine(r.calls, Vec2{10, 95}, Vec2{110, 95}));  // the divider runs the branch's full width
}

TEST(DrawConditional, ScopeFillsItsOwnHeaderBandAndTagsIt) {
  RecordingRenderer r;
  const Viewport viewport;
  const fluir::pt::Conditional conditional = makeConditional();

  fluir::editor::draw::drawScope(
    *conditional.thenScope, fluir::editor::draw::THEN_TAG, Rect{10, 35, 100, 60}, rootView(r, viewport), kCtx);

  EXPECT_TRUE(hasFill(r.calls, Rect{10, 35, 100, kHeaderH}));
  const std::vector<std::string> texts = textStrings(r.calls);
  EXPECT_NE(std::find(texts.begin(), texts.end(), std::string{fluir::editor::draw::THEN_TAG}), texts.end());
}

TEST(DrawConditional, TheElseBranchTagsItsOwnHeaderToo) {
  RecordingRenderer r;
  const Viewport viewport;
  const fluir::pt::Conditional conditional = makeConditional();

  fluir::editor::draw::drawScope(
    *conditional.elseScope, fluir::editor::draw::ELSE_TAG, Rect{10, 95, 100, 30}, rootView(r, viewport), kCtx);

  EXPECT_TRUE(hasFill(r.calls, Rect{10, 95, 100, kHeaderH}));
  const std::vector<std::string> texts = textStrings(r.calls);
  EXPECT_NE(std::find(texts.begin(), texts.end(), std::string{fluir::editor::draw::ELSE_TAG}), texts.end());
}

TEST(DrawConditional, BranchTagNamesWhichBranchAScopeIs) {
  const fluir::pt::Conditional conditional = makeConditional();

  EXPECT_EQ(fluir::editor::draw::branchTag(conditional, *conditional.thenScope), fluir::editor::draw::THEN_TAG);
  EXPECT_EQ(fluir::editor::draw::branchTag(conditional, *conditional.elseScope), fluir::editor::draw::ELSE_TAG);
}

// The `if` chrome is gone: each branch labels itself, so the frame is only a border.
TEST(DrawConditional, FrameDrawsItsBorderAlone) {
  RecordingRenderer r;
  const Viewport viewport;

  fluir::editor::draw::drawFrame(makeConditional(), Rect{10, 35, 100, 90}, rootView(r, viewport), kCtx);

  EXPECT_TRUE(hasRect(r.calls, Rect{10, 35, 100, 90}));  // the border spans both branches
  EXPECT_FALSE(hasFill(r.calls, Rect{10, 35, 100, kHeaderH}));
  EXPECT_TRUE(textStrings(r.calls).empty());
}
