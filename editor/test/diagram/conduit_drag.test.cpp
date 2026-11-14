#include <QGraphicsLineItem>
#include <QLineF>
#include <QPoint>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/location.hpp"
#include "editor/diagram/binary.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/constant.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/parameter.hpp"
#include "editor/diagram/port.hpp"
#include "editor/diagram/return.hpp"
#include "editor/diagram/unary.hpp"
#include "interaction.hpp"

namespace {

  namespace diagram = fluir::editor::diagram;
  namespace pt = fluir::pt;
  using testutil::centerOf;
  using testutil::Interaction;
  using testutil::routed;

  pt::Block& bodyOf(pt::ParseTree& tree, fluir::ID function) {
    return std::get<pt::FunctionDecl>(tree.declarations.at(function)).body;
  }

  pt::Conditional& conditionalOf(pt::ParseTree& tree, fluir::ID function, fluir::ID conditional) {
    return std::get<pt::Conditional>(bodyOf(tree, function).nodes.at(conditional));
  }

  pt::Conduit conduit(fluir::ID id, fluir::ID source, fluir::ID target, int index) {
    return {.id = id, .input = source, .index = 0, .children = {{.target = target, .index = index}}};
  }

  diagram::Terminal& out(const QGraphicsItem& owner, int index = 0) {
    return Interaction::terminalOf(owner, true, index);
  }

  diagram::Terminal& in(const QGraphicsItem& owner, int index = 0) {
    return Interaction::terminalOf(owner, false, index);
  }

  diagram::Port& portOf(diagram::Conditional& conditional, fluir::editor::draw::PortRef ref) {
    for (diagram::Port* port : conditional.wallPorts()) {
      if (port->ref() == ref) {
        return *port;
      }
    }
    throw std::logic_error("no such port");
  }

  std::vector<QGraphicsLineItem*> previews(Interaction& h) { return testutil::all<QGraphicsLineItem>(h.scene()); }

}  // namespace

TEST(ConduitDrag, DraggingAnOutputToAnInputAddsOneUndoableConduit) {
  Interaction h("interaction/unwired.fl");
  const pt::ParseTree before = h.tree();
  pt::ParseTree expected = before;
  bodyOf(expected, 1).conduits.emplace(4, conduit(4, 1, 3, 1));

  h.wire(out(h.find<diagram::Constant>(1)), in(h.find<diagram::Binary>(3), 1));

  EXPECT_EQ(h.tree(), expected);
  EXPECT_TRUE(routed(h.find<diagram::Conduit>(4)));
  ASSERT_EQ(h.stack().count(), 1);
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  h.stack().redo();
  EXPECT_EQ(h.tree(), expected);
}

TEST(ConduitDrag, DraggingAnInputToAnOutputConnectsTheSameWay) {
  Interaction h("interaction/unwired.fl");
  pt::ParseTree expected = h.tree();
  bodyOf(expected, 1).conduits.emplace(4, conduit(4, 2, 3, 0));

  h.wire(in(h.find<diagram::Binary>(3), 0), out(h.find<diagram::Constant>(2)));

  EXPECT_EQ(h.tree(), expected);
}

TEST(ConduitDrag, ATerminalPressNeitherMovesNorResizesItsNode) {
  Interaction h("interaction/unwired.fl");
  auto& binary = h.find<diagram::Binary>(3);
  const fluir::FlowGraphLocation location = binary.location();

  h.wire(in(binary, 0), out(h.find<diagram::Constant>(2)));

  EXPECT_EQ(binary.location(), location);
}

TEST(ConduitDrag, WiringAFedInputMovesItOffItsOldConduitWhichGoesWhenEmpty) {
  Interaction h("interaction/fan_out.fl");
  const pt::ParseTree before = h.tree();
  pt::ParseTree expected = before;
  bodyOf(expected, 1).conduits.erase(6);
  bodyOf(expected, 1).conduits.emplace(7, conduit(7, 1, 2, 1));

  h.wire(out(h.find<diagram::Constant>(1)), in(h.find<diagram::Binary>(2), 1));

  EXPECT_EQ(h.tree(), expected);
  ASSERT_EQ(h.stack().count(), 1);
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  EXPECT_TRUE(routed(h.find<diagram::Conduit>(6)));
}

TEST(ConduitDrag, WiringAFedInputKeepsItsOldConduitsOtherSinks) {
  Interaction h("interaction/fan_out.fl");
  const pt::ParseTree before = h.tree();
  pt::ParseTree expected = before;
  std::erase_if(bodyOf(expected, 1).conduits.at(4).children, [](const auto& sink) { return sink.target == 3; });
  bodyOf(expected, 1).conduits.emplace(7, conduit(7, 5, 3, 0));

  h.wire(out(h.find<diagram::Constant>(5)), in(h.find<diagram::Unary>(3)));

  EXPECT_EQ(h.tree(), expected);
  EXPECT_TRUE(routed(h.find<diagram::Conduit>(4)));
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  EXPECT_TRUE(routed(h.find<diagram::Conduit>(4)));
}

TEST(ConduitDrag, RewiringAnExistingConnectionAddsNothing) {
  Interaction h("interaction/fan_out.fl");
  const pt::ParseTree before = h.tree();

  h.wire(out(h.find<diagram::Constant>(1)), in(h.find<diagram::Binary>(2), 0));

  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(h.stack().count(), 0);
}

TEST(ConduitDrag, AReleaseOffATerminalAddsNothing) {
  Interaction h("interaction/unwired.fl");
  const pt::ParseTree before = h.tree();
  const auto [from, to] = h.aimAt(centerOf(&out(h.find<diagram::Constant>(1))),
                                  h.find<diagram::Binary>(3).sceneBoundingRect().bottomLeft() + QPointF(0, 20));

  h.drag(from, to);

  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(h.stack().count(), 0);
  EXPECT_TRUE(previews(h).empty());
}

TEST(ConduitDrag, AReleaseOnTheSameKindOfTerminalAddsNothing) {
  Interaction h("interaction/unwired.fl");
  const pt::ParseTree before = h.tree();

  h.wire(out(h.find<diagram::Constant>(1)), out(h.find<diagram::Constant>(2)));

  EXPECT_EQ(h.tree(), before);
}

TEST(ConduitDrag, ANodeCannotFeedItself) {
  Interaction h("interaction/unwired.fl");
  const pt::ParseTree before = h.tree();
  auto& binary = h.find<diagram::Binary>(3);

  h.wire(out(binary), in(binary, 0));

  EXPECT_EQ(h.tree(), before);
}

TEST(ConduitDrag, EscapeCancelsTheDrag) {
  Interaction h("interaction/unwired.fl");
  const pt::ParseTree before = h.tree();
  const auto [from, to] =
    h.aimAt(centerOf(&out(h.find<diagram::Constant>(1))), centerOf(&in(h.find<diagram::Binary>(3), 0)));

  h.press(from);
  h.moveTo(to);
  h.key(Qt::Key_Escape);
  EXPECT_TRUE(previews(h).empty());
  h.release(to);

  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(h.stack().count(), 0);
}

TEST(ConduitDrag, DrawsALineFromTheTerminalToTheCursorOnlyWhileDragging) {
  Interaction h("interaction/unwired.fl");
  const diagram::Terminal& source = out(h.find<diagram::Constant>(1));
  const auto [from, to] = h.aimAt(centerOf(&source), centerOf(&source) + QPointF(40, 30));

  h.press(from);
  h.moveTo(to);

  const std::vector<QGraphicsLineItem*> lines = previews(h);
  ASSERT_EQ(lines.size(), 1U);
  const QLineF line = lines.front()->line();
  EXPECT_TRUE(testutil::near(lines.front()->mapToScene(line.p1()), centerOf(&source)));
  EXPECT_TRUE(testutil::near(lines.front()->mapToScene(line.p2()), h.view().mapToScene(to)));

  h.release(to);
  EXPECT_TRUE(previews(h).empty());
}

TEST(ConduitDrag, WiresIntoAFunctionsReturn) {
  Interaction h("interaction/wired_function.fl");
  pt::ParseTree expected = h.tree();
  bodyOf(expected, 1).conduits.erase(7);
  bodyOf(expected, 1).conduits.emplace(10, conduit(10, 5, 9, 0));

  h.wire(out(h.find<diagram::Constant>(5)), in(h.find<diagram::Return>(9)));

  EXPECT_EQ(h.tree(), expected);
}

TEST(ConduitDrag, WiresABodyNodeIntoTheConditionsOuterSide) {
  Interaction h("interaction/ported_conditional.fl");
  pt::ParseTree expected = h.tree();
  bodyOf(expected, 1).conduits.erase(5);
  bodyOf(expected, 1).conduits.emplace(8, conduit(8, 3, 2, 0));
  auto& conditional = h.find<diagram::Conditional>(2);

  h.wire(out(h.find<diagram::Constant>(3)), *portOf(conditional, {}).outer());

  EXPECT_EQ(h.tree(), expected);
}

TEST(ConduitDrag, WiresABodyNodeIntoAnInputPortsOuterSide) {
  Interaction h("interaction/ported_conditional.fl");
  pt::ParseTree expected = h.tree();
  bodyOf(expected, 1).conduits.emplace(8, conduit(8, 1, 2, 1));
  auto& conditional = h.find<diagram::Conditional>(2);

  h.wire(out(h.find<diagram::Constant>(1)), *portOf(conditional, {.output = false, .index = 1}).outer());

  EXPECT_EQ(h.tree(), expected);
}

TEST(ConduitDrag, WiresAPortsInnerSideIntoABranchNodeWithAnIdFreshAcrossBothBranches) {
  Interaction h("interaction/ported_conditional.fl");
  pt::ParseTree expected = h.tree();
  pt::Block& then = *conditionalOf(expected, 1, 2).thenScope;
  then.conduits.erase(6);
  then.conduits.emplace(9, conduit(9, 1, 5, 0));
  auto& conditional = h.find<diagram::Conditional>(2);

  h.wire(*portOf(conditional, {}).inner(), in(h.find<diagram::Unary>(5)));

  EXPECT_EQ(h.tree(), expected);
}

TEST(ConduitDrag, RefusesToWireAcrossAWall) {
  Interaction h("interaction/ported_conditional.fl");
  const pt::ParseTree before = h.tree();
  auto& conditional = h.find<diagram::Conditional>(2);

  h.wire(out(h.find<diagram::Unary>(5)), *portOf(conditional, {}).outer());

  EXPECT_EQ(h.tree(), before);
}

TEST(ConduitDrag, IgnoresTerminalsInTheHiddenBranch) {
  Interaction h("read/conditional_with_body.fl");
  const pt::ParseTree before = h.tree();
  auto& conditional = h.find<diagram::Conditional>(2);
  const diagram::Binary* hidden = nullptr;
  for (QGraphicsItem* child : conditional.elseBranch()->childItems()) {
    if (auto* binary = dynamic_cast<diagram::Binary*>(child)) {
      hidden = binary;
    }
  }
  ASSERT_NE(hidden, nullptr);
  const diagram::Constant* shown = nullptr;
  for (QGraphicsItem* child : conditional.thenBranch()->childItems()) {
    if (auto* constant = dynamic_cast<diagram::Constant*>(child); constant != nullptr && constant->id() == 2) {
      shown = constant;
    }
  }
  ASSERT_NE(shown, nullptr);

  h.wire(out(*shown), in(*hidden, 0));

  EXPECT_EQ(h.tree(), before);
}
