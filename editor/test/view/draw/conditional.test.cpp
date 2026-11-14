#include "editor/view/draw/conditional.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"

// A conditional's pure geometry: ports, tags and size limits.

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::Rect;

  const EditorContext kCtx;
  constexpr double kHeaderH = 25;  // headerUnits 5 * unitPx 5

  // The conditional's size, in grid units.
  constexpr int kWidth = 20;
  constexpr int kHeight = 18;

  fluir::editor::draw::Ports makeConditional() { return {}; }

}  // namespace

TEST(DrawConditional, ColorIsTheConditionalHeaderTheme) {
  EXPECT_EQ(fluir::editor::draw::color(makeConditional(), kCtx.theme), kCtx.theme.conditionalNodeHeader);
}

// One frame, one header band: the frame labels the branch it shows.
TEST(DrawConditional, ConditionPortStraddlesTheLeftWall) {
  const Rect frame{10, 35, 100, 90};

  const Rect port = fluir::editor::draw::portRect(makeConditional(), {}, frame, kCtx.layout);

  EXPECT_DOUBLE_EQ(port.x + port.w / 2, frame.x);
  EXPECT_GT(port.w, 0);
  EXPECT_DOUBLE_EQ(port.w, port.h);
}

// `condition.y` counts grid units from the frame top, header included, to the port's top edge.
TEST(DrawConditional, ConditionPortTopIsItsYBelowTheFrameTop) {
  const Rect frame{10, 35, 100, 90};
  fluir::editor::draw::Ports node = makeConditional();
  node.condition.y = 4;
  const Rect port = fluir::editor::draw::portRect(node, {}, frame, kCtx.layout);
  node.condition.y = 7;
  const Rect lower = fluir::editor::draw::portRect(node, {}, frame, kCtx.layout);

  EXPECT_DOUBLE_EQ(port.y, frame.y + 4 * kCtx.layout.unitPx);
  EXPECT_DOUBLE_EQ(lower.y - port.y, 3 * kCtx.layout.unitPx);
}

// The condition keeps its bool color; every other wall port is neutral.
namespace {

  // Condition at 6, inputs at 10 and 14 (inner 2, 3), outputs at 8 and 12 (inner 4, 5).
  fluir::editor::draw::Ports withWallPorts() {
    fluir::editor::draw::Ports node = makeConditional();
    node.condition = {.innerId = 1, .y = 6};
    node.inputs = {{.innerId = 2, .y = 10}, {.innerId = 3, .y = 14}};
    node.outputs = {{.innerId = 4, .y = 8}, {.innerId = 5, .y = 12}};
    return node;
  }

}  // namespace

TEST(DrawConditional, EveryPortTopIsItsYBelowTheFrameTop) {
  const Rect frame{10, 35, 100, 90};
  const fluir::editor::draw::Ports node = withWallPorts();

  for (const fluir::editor::draw::PortRef ref : fluir::editor::draw::portRefs(node)) {
    const Rect port = fluir::editor::draw::portRect(node, ref, frame, kCtx.layout);
    EXPECT_DOUBLE_EQ(port.y, frame.y + fluir::editor::draw::portOf(node, ref)->y * kCtx.layout.unitPx);
    EXPECT_GT(port.w, 0);
    EXPECT_DOUBLE_EQ(port.w, port.h);
  }
}

TEST(DrawConditional, InputPortsStraddleTheLeftWallAndOutputPortsTheRight) {
  const Rect frame{10, 35, 100, 90};
  const fluir::editor::draw::Ports node = withWallPorts();

  const Rect input = fluir::editor::draw::portRect(node, {.output = false, .index = 2}, frame, kCtx.layout);
  EXPECT_DOUBLE_EQ(input.x + input.w / 2, frame.x);
  const Rect output = fluir::editor::draw::portRect(node, {.output = true, .index = 1}, frame, kCtx.layout);
  EXPECT_DOUBLE_EQ(output.x + output.w / 2, frame.x + frame.w);
}

namespace {

  Rect frameOf(int height) {
    const double unit = kCtx.layout.unitPx;
    return Rect{0, 0, kWidth * unit, height * unit};
  }

}  // namespace

TEST(DrawConditional, PortYLimitsKeepThePortBetweenHeaderAndBottom) {
  fluir::editor::draw::Ports node = makeConditional();
  const Rect frame = frameOf(kHeight);
  const auto limits = fluir::editor::draw::portYLimits(kHeight, kCtx.layout);

  node.condition.y = limits.lower;
  EXPECT_DOUBLE_EQ(fluir::editor::draw::portRect(node, {}, frame, kCtx.layout).y, frame.y + kCtx.layout.headerH());
  node.condition.y = limits.upper;
  const Rect port = fluir::editor::draw::portRect(node, {}, frame, kCtx.layout);
  EXPECT_DOUBLE_EQ(port.y + port.h, frame.y + frame.h);
}

TEST(DrawConditional, SizeLimitsKeepTheConditionPortInsideTheFrame) {
  fluir::editor::draw::Ports node = makeConditional();
  node.condition.y = 30;
  const Rect frame = frameOf(fluir::editor::draw::sizeLimits(node).lower.y);
  const Rect port = fluir::editor::draw::portRect(node, {}, frame, kCtx.layout);
  EXPECT_DOUBLE_EQ(port.y + port.h, frame.y + frame.h);
}

TEST(DrawConditional, SizeLimitsFollowTheLowestPortOnAnyWall) {
  fluir::editor::draw::Ports node = makeConditional();
  node.condition.y = 30;
  const int conditionFloor = fluir::editor::draw::sizeLimits(node).lower.y;

  node.inputs = {{.innerId = 2, .y = 34}};
  EXPECT_EQ(fluir::editor::draw::sizeLimits(node).lower.y, conditionFloor + 4);
  node.outputs = {{.innerId = 3, .y = 40}};
  EXPECT_EQ(fluir::editor::draw::sizeLimits(node).lower.y, conditionFloor + 10);
}
