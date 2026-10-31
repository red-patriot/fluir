#include <QGraphicsItem>
#include <QList>
#include <QPoint>

#include <gtest/gtest.h>

#include "editor/diagram/binary.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/constant.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/move_grip.hpp"
#include "editor/diagram/parameter.hpp"
#include "editor/diagram/port.hpp"
#include "editor/diagram/resize_grip.hpp"
#include "interaction.hpp"

// One thing is selected at a time: a node, or a conduit. A press on a part selects the part's owner.

namespace {

  namespace diagram = fluir::editor::diagram;
  using testutil::Interaction;

  QList<QGraphicsItem*> only(QGraphicsItem& item) { return {&item}; }

}  // namespace

TEST(Selection, APressOnANodeSelectsOnlyIt) {
  Interaction h("read/simple_binary_expr.fl");
  auto& first = h.find<diagram::Constant>(2);
  auto& second = h.find<diagram::Constant>(3);

  h.click(h.body(first));
  EXPECT_EQ(h.scene().selectedItems(), only(first));

  h.click(h.body(second));
  EXPECT_EQ(h.scene().selectedItems(), only(second));
}

TEST(Selection, APressOnAnEmptyBodySelectsItsFunction) {
  Interaction h("read/simple_binary_expr.fl");
  auto& function = h.find<diagram::Function>(1);

  h.click(h.body(function));

  EXPECT_EQ(h.scene().selectedItems(), only(function));
}

TEST(Selection, APressOnEmptyCanvasClearsTheSelection) {
  Interaction h("read/simple_binary_expr.fl");
  h.select(h.find<diagram::Binary>(1));
  ASSERT_FALSE(h.scene().selectedItems().isEmpty());

  h.click(h.aimAt(h.find<diagram::Function>(1).sceneBoundingRect().topLeft() - QPointF(20, 20)));

  EXPECT_TRUE(h.scene().selectedItems().isEmpty());
}

TEST(Selection, EscapeClearsTheSelection) {
  Interaction h("read/simple_binary_expr.fl");
  h.select(h.find<diagram::Binary>(1));
  ASSERT_FALSE(h.scene().selectedItems().isEmpty());

  h.key(Qt::Key_Escape);

  EXPECT_TRUE(h.scene().selectedItems().isEmpty());
}

TEST(Selection, ADragOnEmptyCanvasSelectsNothing) {
  Interaction h("read/simple_binary_expr.fl");
  const QPoint from = h.aimAt(h.find<diagram::Function>(1).sceneBoundingRect().topLeft() - QPointF(20, 20));

  h.drag(from, h.by(from, 200, 200));

  EXPECT_TRUE(h.scene().selectedItems().isEmpty());
}

TEST(Selection, APressOnAParameterSelectsItsFunction) {
  Interaction h("read/function_with_input_only.fl");

  h.click(h.aim(h.find<diagram::Parameter>(2)));

  EXPECT_EQ(h.scene().selectedItems(), only(h.find<diagram::Function>(1)));
}

TEST(Selection, APressOnAPortSelectsItsConditional) {
  Interaction h("write/conditional_with_port.fl");
  auto& conditional = h.find<diagram::Conditional>(2);

  h.click(h.aim(*conditional.wallPorts().front()));

  EXPECT_EQ(h.scene().selectedItems(), only(conditional));
}

TEST(Selection, APressOnAConditionalsHeaderSelectsIt) {
  Interaction h("write/conditional_with_port.fl");
  auto& conditional = h.find<diagram::Conditional>(2);
  const QRectF frame = conditional.sceneBoundingRect();

  h.click(h.aim(conditional, QPointF(0, h.ctx().layout.headerH() / 2 - frame.height() / 2)));

  EXPECT_EQ(h.scene().selectedItems(), only(conditional));
}

TEST(Selection, APressOnAConduitSelectsIt) {
  Interaction h("interaction/fan_out.fl");
  auto& conduit = h.find<diagram::Conduit>(6);

  h.click(h.aim(conduit));

  EXPECT_EQ(h.scene().selectedItems(), only(conduit));
}

TEST(Selection, APressInAConduitsBoundsButOffItsLinesMissesIt) {
  Interaction h("interaction/fan_out.fl");
  auto& conduit = h.find<diagram::Conduit>(4);
  const QPointF source = testutil::centerOf(conduit.source());
  const QPointF lowest =
    testutil::centerOf(conduit.sinks().front()).y() > testutil::centerOf(conduit.sinks().back()).y() ?
      testutil::centerOf(conduit.sinks().front()) :
      testutil::centerOf(conduit.sinks().back());
  const QPointF corner(source.x() + 2 * h.unit(), lowest.y());
  ASSERT_TRUE(conduit.sceneBoundingRect().contains(corner));

  h.click(h.aimAt(corner));

  EXPECT_FALSE(conduit.isSelected());
}

TEST(Selection, GripsShowWhetherOrNotTheirNodeIsSelected) {
  Interaction h("read/simple_binary_expr.fl");
  auto& binary = h.find<diagram::Binary>(1);
  const auto& move = Interaction::part<diagram::MoveGrip>(binary);
  const auto& resize = Interaction::part<diagram::ResizeGrip>(binary);
  EXPECT_TRUE(move.isVisible());
  EXPECT_TRUE(resize.isVisible());

  h.select(binary);
  EXPECT_TRUE(move.isVisible());
  EXPECT_TRUE(resize.isVisible());
}

TEST(Selection, APressOnAGripSelectsItsNode) {
  Interaction h("read/simple_binary_expr.fl");
  auto& binary = h.find<diagram::Binary>(1);
  h.click(h.body(h.find<diagram::Constant>(2)));

  h.click(h.aim(Interaction::part<diagram::MoveGrip>(binary)));
  EXPECT_EQ(h.scene().selectedItems(), only(binary));

  h.click(h.body(h.find<diagram::Constant>(2)));
  h.click(h.aim(Interaction::part<diagram::ResizeGrip>(binary)));
  EXPECT_EQ(h.scene().selectedItems(), only(binary));
}
