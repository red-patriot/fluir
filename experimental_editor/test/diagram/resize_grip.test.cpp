#include "editor/diagram/resize_grip.hpp"

#include <filesystem>
#include <optional>
#include <QPoint>
#include <QPointF>

#include <gtest/gtest.h>

#include "compiler/models/location.hpp"
#include "editor/diagram/binary.hpp"
#include "editor/diagram/call.hpp"
#include "editor/diagram/comment.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/constant.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/node.hpp"
#include "editor/diagram/port.hpp"
#include "editor/diagram/return.hpp"
#include "editor/diagram/unary.hpp"
#include "editor/view/draw/binary.hpp"
#include "editor/view/draw/call.hpp"
#include "editor/view/draw/comment.hpp"
#include "editor/view/draw/conditional.hpp"
#include "editor/view/draw/constant.hpp"
#include "editor/view/draw/function.hpp"
#include "editor/view/draw/unary.hpp"
#include "interaction.hpp"

// A resize grip drag sizes its node live in whole grid units, along its kind's axes and within its kind's limits.

namespace {

  namespace fs = std::filesystem;
  namespace diagram = fluir::editor::diagram;
  namespace draw = fluir::editor::draw;
  using fluir::FlowGraphLocation;
  using fluir::editor::ResizeAxes;
  using testutil::Interaction;
  using testutil::routed;

  QPoint grabResize(Interaction& h, diagram::FluirNode& node) {
    return h.aim(Interaction::part<diagram::ResizeGrip>(node));
  }

  FlowGraphLocation sizedBy(FlowGraphLocation location, int w, int h) {
    location.width += w;
    location.height += h;
    return location;
  }

  // The axes `node`'s kind resizes along, if any.
  std::optional<ResizeAxes> axesOf(const diagram::FluirNode& node) {
    if (const auto* n = dynamic_cast<const diagram::Constant*>(&node)) {
      return draw::resizePart(n->data());
    }
    if (const auto* n = dynamic_cast<const diagram::Binary*>(&node)) {
      return draw::resizePart(n->data());
    }
    if (const auto* n = dynamic_cast<const diagram::Unary*>(&node)) {
      return draw::resizePart(n->data());
    }
    if (const auto* n = dynamic_cast<const diagram::Call*>(&node)) {
      return draw::resizePart(n->data());
    }
    if (const auto* n = dynamic_cast<const diagram::Comment*>(&node)) {
      return draw::resizePart(n->data());
    }
    if (const auto* n = dynamic_cast<const diagram::Conditional*>(&node)) {
      return draw::resizePart(n->ports());
    }
    return ResizeAxes::XY;  // a function
  }

}  // namespace

class ResizeGripOnEachFixture : public testing::TestWithParam<fs::path> { };

INSTANTIATE_TEST_SUITE_P(Fixtures,
                         ResizeGripOnEachFixture,
                         testing::ValuesIn(testutil::readWriteFixtures(true)),
                         testutil::fixtureName);

TEST_P(ResizeGripOnEachFixture, ANodeHasAGripExactlyWhenItsKindResizes) {
  Interaction h(GetParam().string());

  for (const diagram::FluirNode* node : testutil::all<diagram::FluirNode>(h.scene())) {
    const std::optional<ResizeAxes> axes = axesOf(*node);
    int grips = 0;
    for (const QGraphicsItem* child : node->childItems()) {
      if (const auto* grip = dynamic_cast<const diagram::ResizeGrip*>(child)) {
        ++grips;
        EXPECT_EQ(std::optional{grip->axes()}, axes) << "node " << node->id();
      }
    }
    EXPECT_EQ(grips, axes ? 1 : 0) << "node " << node->id();
  }
}

TEST(ResizeGrip, AnXGripGrowsWidthOnlyLive) {
  Interaction h("read/simple_binary_expr.fl");
  auto& constant = h.find<diagram::Constant>(2);
  const FlowGraphLocation start = constant.location();
  const QPoint from = grabResize(h, constant);

  h.press(from);
  h.moveTo(h.by(from, 3, 4));

  EXPECT_EQ(constant.location(), sizedBy(start, 3, 0)) << "a resize never moves";
  EXPECT_EQ(h.stack().count(), 0) << "nothing is pushed until release";
  h.release(h.by(from, 3, 4));
}

TEST(ResizeGrip, AnXYGripResizesBothAxes) {
  Interaction h("read/top_level_comment_only.fl");
  auto& comment = h.find<diagram::Comment>(1);
  const FlowGraphLocation start = comment.location();
  const QPoint from = grabResize(h, comment);

  h.drag(from, h.by(from, 3, 2));

  EXPECT_EQ(comment.location(), sizedBy(start, 3, 2));
}

TEST(ResizeGrip, ANodeClampsToItsKindsLimits) {
  Interaction h("read/simple_binary_expr.fl");
  auto& constant = h.find<diagram::Constant>(2);
  const auto limits = draw::sizeLimits(constant.data());
  const QPoint from = grabResize(h, constant);
  h.press(from);

  h.moveTo(h.by(from, -100, 0));
  EXPECT_EQ(constant.location().width, limits.lower.x);
  h.moveTo(h.by(from, limits.upper.x + 100, 0));
  EXPECT_EQ(constant.location().width, limits.upper.x);
  h.release(h.by(from, limits.upper.x + 100, 0));
}

TEST(ResizeGrip, ACommentClampsInBothAxes) {
  Interaction h("read/top_level_comment_only.fl");
  auto& comment = h.find<diagram::Comment>(1);
  const auto limits = draw::sizeLimits(comment.data());
  const QPoint from = grabResize(h, comment);

  h.drag(from, h.by(from, -100, -100));

  EXPECT_EQ(comment.location().width, limits.lower.x);
  EXPECT_EQ(comment.location().height, limits.lower.y);
}

TEST(ResizeGrip, AConditionalCannotShrinkPastItsLowestPort) {
  Interaction h("write/conditional_with_port.fl");
  auto& conditional = h.find<diagram::Conditional>(2);
  const auto limits = draw::sizeLimits(conditional.ports());
  const QPoint from = grabResize(h, conditional);

  h.drag(from, h.by(from, -1000, -1000));

  EXPECT_EQ(conditional.location().width, limits.lower.x);
  EXPECT_EQ(conditional.location().height, limits.lower.y);
}

// Limits come from the kind alone: a shrunk container may clip its children, which stay where they are.
TEST(ResizeGrip, AFunctionShrinksPastItsBodyWithoutMovingIt) {
  Interaction h("read/simple_binary_expr.fl");
  auto& function = h.find<diagram::Function>(1);
  auto& binary = h.find<diagram::Binary>(1);
  const auto* scope = binary.parentItem();
  const FlowGraphLocation binaryStart = binary.location();
  const auto limits = draw::sizeLimits(function.signature());
  const QPoint from = grabResize(h, function);

  h.drag(from, h.by(from, -1000, -1000));

  EXPECT_EQ(function.location().width, limits.lower.x);
  EXPECT_EQ(function.location().height, limits.lower.y);
  EXPECT_EQ(binary.location(), binaryStart);
  EXPECT_EQ(binary.parentItem(), scope);
}

TEST(ResizeGrip, ReleasePushesOneUndoableCommandForTheWholeGesture) {
  Interaction h("read/top_level_comment_only.fl");
  const fluir::pt::ParseTree before = h.tree();
  const QPoint from = grabResize(h, h.find<diagram::Comment>(1));

  h.press(from);
  h.moveTo(h.by(from, 1, 1));
  h.moveTo(h.by(from, 3, 2));
  h.release(h.by(from, 3, 2));
  const fluir::pt::ParseTree after = h.tree();

  ASSERT_EQ(h.stack().count(), 1);
  EXPECT_NE(after, before);
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  h.stack().redo();
  EXPECT_EQ(h.tree(), after);
}

TEST(ResizeGrip, AGestureThatEndsWhereItStartedPushesNothing) {
  Interaction h("read/top_level_comment_only.fl");
  const QPoint from = grabResize(h, h.find<diagram::Comment>(1));

  h.press(from);
  h.moveTo(h.by(from, 3, 2));
  h.moveTo(from);
  h.release(from);

  EXPECT_EQ(h.stack().count(), 0);
}

TEST(ResizeGrip, EscapeCancelsALiveDrag) {
  Interaction h("read/top_level_comment_only.fl");
  const fluir::pt::ParseTree before = h.tree();
  const QPoint from = grabResize(h, h.find<diagram::Comment>(1));
  h.press(from);
  h.moveTo(h.by(from, 3, 2));

  h.key(Qt::Key_Escape);
  EXPECT_EQ(h.tree(), before);
  h.release(h.by(from, 3, 2));

  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(h.stack().count(), 0);
}

TEST(ResizeGrip, AReturnRailKeepsToTheFunctionsTopRight) {
  Interaction h("read/function_with_output_only.fl");
  auto& function = h.find<diagram::Function>(1);
  const auto& rail = h.find<diagram::Return>(4);
  const QPointF offset = rail.sceneBoundingRect().topRight() - function.sceneBoundingRect().topRight();
  const QPoint from = grabResize(h, function);

  h.drag(from, h.by(from, 4, 2));

  EXPECT_EQ(rail.sceneBoundingRect().topRight() - function.sceneBoundingRect().topRight(), offset);
}

TEST(ResizeGrip, OutputPortsAndTheirConduitsFollowTheRightWall) {
  Interaction h("write/conditional_with_port.fl");
  auto& conditional = h.find<diagram::Conditional>(2);
  const QPoint from = grabResize(h, conditional);

  h.drag(from, h.by(from, 4, 0));

  for (const diagram::Port* port : conditional.wallPorts()) {
    const double wall =
      port->ref().output ? conditional.sceneBoundingRect().right() : conditional.sceneBoundingRect().left();
    EXPECT_DOUBLE_EQ(port->sceneBoundingRect().center().x(), wall);
  }
  EXPECT_TRUE(routed(h.find<diagram::Conduit>(9)));
}
