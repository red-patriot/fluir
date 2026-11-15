#include <algorithm>
#include <QPoint>
#include <QPointF>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/diagram/binary.hpp"
#include "editor/diagram/comment.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/constant.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/move_grip.hpp"
#include "editor/diagram/unary.hpp"
#include "interaction.hpp"

// Delete removes the selection as one command. A node takes the conduits it sources, drops out of the conduits it
// sinks, and a conduit left with no sinks goes too.
// fan_out.fl: constant 1 feeds binary 2 and unary 3 through conduit 4; constant 5 feeds binary 2 through conduit 6.

namespace {

  namespace diagram = fluir::editor::diagram;
  using testutil::Interaction;
  using testutil::routed;

  fluir::pt::Block& bodyOf(fluir::pt::ParseTree& tree, fluir::ID function) {
    return std::get<fluir::pt::FunctionDecl>(tree.declarations.at(function)).body;
  }

  void dropSink(fluir::pt::Conduit& conduit, fluir::ID target) {
    std::erase_if(conduit.children, [target](const auto& output) { return output.target == target; });
  }

  void selectAndDelete(Interaction& h, const diagram::FluirNode& node) {
    h.select(node);
    h.key(Qt::Key_Delete);
  }

}  // namespace

TEST(Delete, ANodeTakesTheConduitsLeftWithNoSinks) {
  Interaction h("read/simple_binary_expr.fl");
  fluir::pt::ParseTree expected = h.tree();
  bodyOf(expected, 1).nodes.erase(1);
  bodyOf(expected, 1).conduits.clear();

  selectAndDelete(h, h.find<diagram::Binary>(1));

  EXPECT_EQ(h.tree(), expected);
}

TEST(Delete, ASourceTakesItsConduits) {
  Interaction h("interaction/fan_out.fl");
  fluir::pt::ParseTree expected = h.tree();
  bodyOf(expected, 1).nodes.erase(1);
  bodyOf(expected, 1).conduits.erase(4);

  selectAndDelete(h, h.find<diagram::Constant>(1));

  EXPECT_EQ(h.tree(), expected);
}

TEST(Delete, ASinkLeavesItsConduitsOtherSinks) {
  Interaction h("interaction/fan_out.fl");
  fluir::pt::ParseTree expected = h.tree();
  bodyOf(expected, 1).nodes.erase(3);
  dropSink(bodyOf(expected, 1).conduits.at(4), 3);

  selectAndDelete(h, h.find<diagram::Unary>(3));

  EXPECT_EQ(h.tree(), expected);
  EXPECT_TRUE(routed(h.find<diagram::Conduit>(4)));
}

TEST(Delete, ASinkTakesAConduitLeftWithNoSinks) {
  Interaction h("interaction/fan_out.fl");
  fluir::pt::ParseTree expected = h.tree();
  bodyOf(expected, 1).nodes.erase(2);
  bodyOf(expected, 1).conduits.erase(6);
  dropSink(bodyOf(expected, 1).conduits.at(4), 2);

  selectAndDelete(h, h.find<diagram::Binary>(2));

  EXPECT_EQ(h.tree(), expected);
}

TEST(Delete, ASelectedConduitGoesAlone) {
  Interaction h("interaction/fan_out.fl");
  fluir::pt::ParseTree expected = h.tree();
  bodyOf(expected, 1).conduits.erase(6);

  h.click(h.aim(h.find<diagram::Conduit>(6)));
  h.key(Qt::Key_Delete);

  EXPECT_EQ(h.tree(), expected);
}

TEST(Delete, AConditionalTakesItsBranches) {
  Interaction h("write/conditional_with_port.fl");
  fluir::pt::ParseTree expected = h.tree();
  bodyOf(expected, 1).nodes.erase(2);
  auto& conditional = h.find<diagram::Conditional>(2);
  const QRectF frame = conditional.sceneBoundingRect();

  h.click(h.aim(conditional, QPointF(0, h.ctx().layout.headerH() / 2 - frame.height() / 2)));
  h.key(Qt::Key_Delete);

  EXPECT_EQ(h.tree(), expected);
}

TEST(Delete, AFunctionTakesItsDeclaration) {
  Interaction h("read/multiple_empty_functions.fl");
  fluir::pt::ParseTree expected = h.tree();
  expected.declarations.erase(7);

  selectAndDelete(h, h.find<diagram::Function>(7));

  EXPECT_EQ(h.tree(), expected);
}

TEST(Delete, ATopLevelCommentGoes) {
  Interaction h("read/top_level_comment_only.fl");

  selectAndDelete(h, h.find<diagram::Comment>(1));

  EXPECT_TRUE(h.tree().declarations.empty());
}

TEST(Delete, NothingSelectedDeletesNothing) {
  Interaction h("interaction/fan_out.fl");
  const fluir::pt::ParseTree before = h.tree();

  h.key(Qt::Key_Delete);

  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(h.stack().count(), 0);
}

TEST(Delete, OneDeleteIsOneUndoableCommand) {
  Interaction h("interaction/fan_out.fl");
  const fluir::pt::ParseTree before = h.tree();
  selectAndDelete(h, h.find<diagram::Binary>(2));
  const fluir::pt::ParseTree after = h.tree();

  ASSERT_EQ(h.stack().count(), 1);
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  for (const diagram::Conduit* conduit : testutil::all<diagram::Conduit>(h.scene())) {
    EXPECT_TRUE(routed(*conduit)) << "conduit " << conduit->id();
  }
  h.stack().redo();
  EXPECT_EQ(h.tree(), after);
}

// Earlier commands still name the deleted items, so undo must bring back the same items.
TEST(Delete, UndoingADeleteLetsAnEarlierEditUndo) {
  Interaction h("interaction/fan_out.fl");
  const fluir::pt::ParseTree before = h.tree();
  auto& binary = h.find<diagram::Binary>(2);
  const QPoint grip = h.aim(Interaction::part<diagram::MoveGrip>(binary));
  h.drag(grip, h.by(grip, 2, 1));
  selectAndDelete(h, binary);
  ASSERT_EQ(h.stack().count(), 2);

  h.stack().undo();
  h.stack().undo();

  EXPECT_EQ(h.tree(), before);
}
