#include "editor/diagram/move_grip.hpp"

#include <QPoint>
#include <QPointF>

#include <gtest/gtest.h>

#include "compiler/models/location.hpp"
#include "editor/diagram/binary.hpp"
#include "editor/diagram/comment.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/constant.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/node.hpp"
#include "interaction.hpp"

// A move grip drag moves its node live in whole grid units and pushes one command on release.

namespace {

  namespace diagram = fluir::editor::diagram;
  using fluir::FlowGraphLocation;
  using testutil::Interaction;
  using testutil::routed;

  QPoint grabMove(Interaction& h, diagram::FluirNode& node) {
    return h.aim(Interaction::part<diagram::MoveGrip>(node));
  }

  FlowGraphLocation movedBy(FlowGraphLocation location, int x, int y) {
    location.x += x;
    location.y += y;
    return location;
  }

}  // namespace

TEST(MoveGrip, ADragMovesItsNodeLiveInWholeUnits) {
  Interaction h("read/simple_binary_expr.fl");
  auto& binary = h.find<diagram::Binary>(1);
  const FlowGraphLocation start = binary.location();
  const QPoint from = grabMove(h, binary);

  h.press(from);
  h.moveTo(h.by(from, 2, 1));

  EXPECT_EQ(binary.location(), movedBy(start, 2, 1));
  EXPECT_EQ(h.stack().count(), 0) << "nothing is pushed until release";
  h.release(h.by(from, 2, 1));
}

TEST(MoveGrip, SubUnitMotionCarriesOver) {
  Interaction h("read/simple_binary_expr.fl");
  auto& binary = h.find<diagram::Binary>(1);
  const FlowGraphLocation start = binary.location();
  const QPoint from = grabMove(h, binary);
  h.press(from);

  h.moveTo(h.by(from, 0, 0, QPoint(3, 0)));
  EXPECT_EQ(binary.location(), start);
  h.moveTo(h.by(from, 0, 0, QPoint(6, 0)));
  EXPECT_EQ(binary.location(), movedBy(start, 1, 0));
  h.release(h.by(from, 0, 0, QPoint(6, 0)));
}

TEST(MoveGrip, ReleasePushesOneUndoableCommandForTheWholeGesture) {
  Interaction h("read/simple_binary_expr.fl");
  auto& binary = h.find<diagram::Binary>(1);
  const fluir::pt::ParseTree before = h.tree();
  const QPoint from = grabMove(h, binary);

  h.press(from);
  h.moveTo(h.by(from, 1, 0));
  h.moveTo(h.by(from, 2, 1));
  h.release(h.by(from, 2, 1));
  const fluir::pt::ParseTree after = h.tree();

  ASSERT_EQ(h.stack().count(), 1);
  EXPECT_NE(after, before);
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  h.stack().redo();
  EXPECT_EQ(h.tree(), after);
}

TEST(MoveGrip, AGestureThatEndsWhereItStartedPushesNothing) {
  Interaction h("read/simple_binary_expr.fl");
  const fluir::pt::ParseTree before = h.tree();
  const QPoint from = grabMove(h, h.find<diagram::Binary>(1));

  h.press(from);
  h.moveTo(h.by(from, 2, 0));
  h.moveTo(from);
  h.release(from);

  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(h.stack().count(), 0);
}

TEST(MoveGrip, EscapeCancelsALiveDrag) {
  Interaction h("read/simple_binary_expr.fl");
  auto& binary = h.find<diagram::Binary>(1);
  const fluir::pt::ParseTree before = h.tree();
  const QPoint from = grabMove(h, binary);
  h.press(from);
  h.moveTo(h.by(from, 2, 0));

  h.key(Qt::Key_Escape);
  EXPECT_EQ(h.tree(), before);
  h.release(h.by(from, 2, 0));

  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(h.stack().count(), 0);
}

TEST(MoveGrip, ConduitsFollowAMovedNode) {
  Interaction h("read/simple_binary_expr.fl");
  auto& conduit = h.find<diagram::Conduit>(4);
  const QPoint from = grabMove(h, h.find<diagram::Constant>(2));

  h.press(from);
  h.moveTo(h.by(from, 3, 4));
  EXPECT_TRUE(routed(conduit)) << "during the drag";
  h.release(h.by(from, 3, 4));
  EXPECT_TRUE(routed(conduit)) << "after release";

  h.stack().undo();
  EXPECT_TRUE(routed(conduit)) << "after undo";
}

TEST(MoveGrip, AFunctionCarriesItsBody) {
  Interaction h("read/simple_binary_expr.fl");
  auto& function = h.find<diagram::Function>(1);
  auto& binary = h.find<diagram::Binary>(1);
  const FlowGraphLocation start = function.location();
  const FlowGraphLocation binaryStart = binary.location();
  const QPointF binaryAt = binary.scenePos();
  const QPoint from = grabMove(h, function);

  h.drag(from, h.by(from, 2, 1));

  EXPECT_EQ(function.location(), movedBy(start, 2, 1));
  EXPECT_EQ(binary.location(), binaryStart) << "body locations are relative to the body";
  EXPECT_EQ(binary.scenePos(), binaryAt + QPointF(2 * h.unit(), h.unit()));
  EXPECT_TRUE(routed(h.find<diagram::Conduit>(4)));
}

TEST(MoveGrip, ANestedNodeMovesInsideItsBranch) {
  Interaction h("read/conditional_with_body.fl");
  auto& binary = h.find<diagram::Binary>(16);
  const auto* branch = binary.parentItem();
  const FlowGraphLocation start = binary.location();
  const QPoint from = grabMove(h, binary);

  h.drag(from, h.by(from, 2, 1));

  EXPECT_EQ(binary.location(), movedBy(start, 2, 1));
  EXPECT_EQ(binary.parentItem(), branch);
}

// Moves snap but do not clamp: a node can leave its scope's visible area.
TEST(MoveGrip, ANodeCanMovePastItsScopesEdge) {
  Interaction h("read/simple_binary_expr.fl");
  auto& binary = h.find<diagram::Binary>(1);
  const auto* scope = binary.parentItem();
  const FlowGraphLocation start = binary.location();
  const QPoint from = grabMove(h, binary);

  h.drag(from, h.by(from, -(start.x + 5), 0));

  EXPECT_EQ(binary.location().x, -5);
  EXPECT_EQ(binary.parentItem(), scope);
}

TEST(MoveGrip, DeltasAreMeasuredInSceneUnits) {
  Interaction h("read/simple_binary_expr.fl");
  auto& binary = h.find<diagram::Binary>(1);
  const FlowGraphLocation start = binary.location();
  h.view().scale(2, 2);
  const QPoint from = grabMove(h, binary);

  h.drag(from, h.by(from, 4, 0));

  EXPECT_EQ(binary.location(), movedBy(start, 2, 0));
}

TEST(MoveGrip, ATopLevelCommentMoves) {
  Interaction h("read/top_level_comment_only.fl");
  auto& comment = h.find<diagram::Comment>(1);
  const FlowGraphLocation start = comment.location();
  const QPoint from = grabMove(h, comment);

  h.drag(from, h.by(from, 2, 1));

  EXPECT_EQ(comment.location(), movedBy(start, 2, 1));
  ASSERT_EQ(h.stack().count(), 1);
}
