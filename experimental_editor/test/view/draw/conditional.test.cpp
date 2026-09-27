#include "editor/view/draw/conditional.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/tree.hpp"
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

  fluir::editor::et::Conditional makeConditional() {
    return fluir::editor::et::Conditional{.id = 20,
                                          .location = {.x = 0, .y = 0, .z = 0, .width = 20, .height = 18},
                                          .condition = {},
                                          .inputs = {},
                                          .outputs = {},
                                          .thenScope = xyz::indirect<fluir::editor::et::Block>{},
                                          .elseScope = xyz::indirect<fluir::editor::et::Block>{}};
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

namespace {

  bool inside(fluir::editor::Vec2 p, const Rect& r) {
    return p.x >= r.x && p.x <= r.x + r.w && p.y >= r.y && p.y <= r.y + r.h;
  }

  fluir::editor::et::Conditional withConditionPort(fluir::ID innerId) {
    fluir::editor::et::Conditional node = makeConditional();
    node.condition = {.innerId = innerId, .y = 10};
    return node;
  }

}  // namespace

// Outside, the condition is the conditional's own input 0, anchored on the port left of the wall.
TEST(DrawConditional, AnchorsAreTheConditionPortOutsideTheWall) {
  const Rect frame{10, 35, 100, 90};
  const fluir::editor::et::Conditional node = withConditionPort(5);

  const fluir::editor::TerminalSet terminals = fluir::editor::draw::anchors(node, frame, kCtx.layout);

  ASSERT_EQ(terminals.inputs.size(), 1u);
  EXPECT_TRUE(terminals.outputs.empty());
  EXPECT_TRUE(inside(terminals.inputs[0], fluir::editor::draw::conditionPortRect(node, frame, kCtx.layout)));
  EXPECT_LT(terminals.inputs[0].x, frame.x);
}

// Inside, the condition feeds its branch as an output keyed by the port's inner id, right of the wall.
TEST(DrawConditional, InnerAnchorsAreTheConditionPortInsideTheWall) {
  const Rect frame{10, 35, 100, 90};
  const fluir::editor::et::Conditional node = withConditionPort(5);

  const auto inner = fluir::editor::draw::innerAnchors(node, frame, kCtx.layout);

  ASSERT_EQ(inner.size(), 1u);
  ASSERT_TRUE(inner.contains(5));
  const fluir::editor::TerminalSet& port = inner.at(5);
  ASSERT_EQ(port.outputs.size(), 1u);
  EXPECT_TRUE(port.inputs.empty());
  EXPECT_TRUE(inside(port.outputs[0], fluir::editor::draw::conditionPortRect(node, frame, kCtx.layout)));
  EXPECT_GT(port.outputs[0].x, frame.x);
}

TEST(DrawConditional, AnUnsetInnerIdHasNoInnerAnchors) {
  EXPECT_TRUE(
    fluir::editor::draw::innerAnchors(withConditionPort(fluir::INVALID_ID), Rect{10, 35, 100, 90}, kCtx.layout)
      .empty());
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

// The tag is the annotation's, not a fixed one: that is all the header says about which branch is shown.
TEST(DrawConditional, FrameTagsTheBranchTheAnnotationShows) {
  RecordingRenderer r;
  const Viewport viewport;
  fluir::editor::et::Conditional node = makeConditional();
  node.annotation.shownBranch = ELSE_BRANCH_ID;

  fluir::editor::draw::drawFrame(node, Rect{10, 35, 100, 90}, rootView(r, viewport), kCtx);

  EXPECT_TRUE(hasText(r, fluir::editor::draw::ELSE_TAG));
  EXPECT_FALSE(hasText(r, fluir::editor::draw::THEN_TAG));
}

// The port sits on the wall, half outside the frame and half in.
TEST(DrawConditional, ConditionPortStraddlesTheLeftWall) {
  const Rect frame{10, 35, 100, 90};

  const Rect port = fluir::editor::draw::conditionPortRect(makeConditional(), frame, kCtx.layout);

  EXPECT_DOUBLE_EQ(port.x + port.w / 2, frame.x);
  EXPECT_GT(port.w, 0);
  EXPECT_DOUBLE_EQ(port.w, port.h);
}

// `condition.y` counts grid units from the frame top, header included, to the port's top edge.
TEST(DrawConditional, ConditionPortTopIsItsYBelowTheFrameTop) {
  const Rect frame{10, 35, 100, 90};
  fluir::editor::et::Conditional node = makeConditional();
  node.condition.y = 4;
  const Rect port = fluir::editor::draw::conditionPortRect(node, frame, kCtx.layout);
  node.condition.y = 7;
  const Rect lower = fluir::editor::draw::conditionPortRect(node, frame, kCtx.layout);

  EXPECT_DOUBLE_EQ(port.y, frame.y + 4 * kCtx.layout.unitPx);
  EXPECT_DOUBLE_EQ(lower.y - port.y, 3 * kCtx.layout.unitPx);
}

TEST(DrawConditional, PortFillsItsRect) {
  RecordingRenderer r;
  const Viewport viewport;

  fluir::editor::draw::drawPort(Rect{5, 50, 15, 15}, rootView(r, viewport), kCtx);

  EXPECT_TRUE(hasFill(r.calls, Rect{5, 50, 15, 15}));
}

namespace {

  Rect frameOf(const fluir::editor::et::Conditional& node) {
    const double unit = kCtx.layout.unitPx;
    return Rect{0, 0, node.location.width * unit, node.location.height * unit};
  }

}  // namespace

TEST(DrawConditional, PortYLimitsKeepThePortBetweenHeaderAndBottom) {
  fluir::editor::et::Conditional node = makeConditional();
  const Rect frame = frameOf(node);
  const auto limits = fluir::editor::draw::portYLimits(node, kCtx.layout);

  node.condition.y = limits.lower;
  EXPECT_DOUBLE_EQ(fluir::editor::draw::conditionPortRect(node, frame, kCtx.layout).y, frame.y + kCtx.layout.headerH());
  node.condition.y = limits.upper;
  const Rect port = fluir::editor::draw::conditionPortRect(node, frame, kCtx.layout);
  EXPECT_DOUBLE_EQ(port.y + port.h, frame.y + frame.h);
}

TEST(DrawConditional, SizeLimitsKeepTheConditionPortInsideTheFrame) {
  fluir::editor::et::Conditional node = makeConditional();
  node.condition.y = 30;
  node.location.height = fluir::editor::draw::sizeLimits(node).lower.y;

  const Rect frame = frameOf(node);
  const Rect port = fluir::editor::draw::conditionPortRect(node, frame, kCtx.layout);
  EXPECT_DOUBLE_EQ(port.y + port.h, frame.y + frame.h);
}

TEST(DrawConditional, SizeLimitsFollowTheLowestPortOnAnyWall) {
  fluir::editor::et::Conditional node = makeConditional();
  node.condition.y = 30;
  const int conditionFloor = fluir::editor::draw::sizeLimits(node).lower.y;

  node.inputs = {{.innerId = 2, .y = 34}};
  EXPECT_EQ(fluir::editor::draw::sizeLimits(node).lower.y, conditionFloor + 4);
  node.outputs = {{.innerId = 3, .y = 40}};
  EXPECT_EQ(fluir::editor::draw::sizeLimits(node).lower.y, conditionFloor + 10);
}
