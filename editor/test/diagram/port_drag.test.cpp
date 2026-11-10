#include <QPoint>
#include <stdexcept>

#include <gtest/gtest.h>

#include "compiler/models/location.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/port.hpp"
#include "editor/view/draw/conditional.hpp"
#include "interaction.hpp"

// A port drag slides the port along its wall live in whole grid units and pushes one command on release.
// conditional_with_port.fl: conditional 2 has condition y 10, inputs y 20 (inner 2) and 10 (inner 21), output y 10.

namespace {

  namespace diagram = fluir::editor::diagram;
  namespace draw = fluir::editor::draw;
  using fluir::FlowGraphLocation;
  using testutil::Interaction;
  using testutil::routed;

  constexpr draw::PortRef kInput{.output = false, .index = 1};
  constexpr draw::PortRef kOutput{.output = true, .index = 0};

  struct Harness : Interaction {
    Harness() : Interaction("write/conditional_with_port.fl") { }

    diagram::Conditional& conditional() { return find<diagram::Conditional>(2); }
    int y(draw::PortRef ref) { return draw::portOf(conditional().ports(), ref)->y; }
    diagram::Port& port(draw::PortRef ref) {
      for (diagram::Port* port : conditional().wallPorts()) {
        if (port->ref() == ref) {
          return *port;
        }
      }
      throw std::logic_error("no such port");
    }
    QPoint grab(draw::PortRef ref) { return aim(port(ref)); }
  };

}  // namespace

TEST(PortDrag, ADragSlidesThePortLiveInWholeUnitsAndIgnoresX) {
  Harness h;
  const int start = h.y(kInput);
  const FlowGraphLocation location = h.conditional().location();
  const QPoint from = h.grab(kInput);

  h.press(from);
  h.moveTo(h.by(from, 10, 2, QPoint(0, 1)));

  EXPECT_EQ(h.y(kInput), start + 2);
  EXPECT_DOUBLE_EQ(h.port(kInput).sceneBoundingRect().top(),
                   h.conditional().sceneBoundingRect().top() + h.y(kInput) * h.unit());
  EXPECT_EQ(h.conditional().location(), location) << "the conditional stays put";
  EXPECT_EQ(h.stack().count(), 0) << "nothing is pushed until release";
  h.release(h.by(from, 10, 2, QPoint(0, 1)));
}

TEST(PortDrag, OnlyThePressedPortMoves) {
  Harness h;
  const draw::Ports before = h.conditional().ports();
  const QPoint from = h.grab(kInput);

  h.drag(from, h.by(from, 0, 3));

  draw::Ports expected = before;
  expected.inputs[0].y += 3;
  EXPECT_EQ(h.conditional().ports(), expected);
}

TEST(PortDrag, ThePortStopsAtItsLimits) {
  Harness h;
  const auto limits = draw::portYLimits(h.conditional().location().height, h.ctx().layout);
  const QPoint from = h.grab(kInput);
  h.press(from);

  h.moveTo(h.by(from, 0, -1000));
  EXPECT_EQ(h.y(kInput), limits.lower);
  h.moveTo(h.by(from, 0, 1000));
  EXPECT_EQ(h.y(kInput), limits.upper);
  h.release(h.by(from, 0, 1000));
}

TEST(PortDrag, ReleasePushesOneUndoableCommandForTheWholeGesture) {
  Harness h;
  const fluir::pt::ParseTree before = h.tree();
  const QPoint from = h.grab(kInput);

  h.press(from);
  h.moveTo(h.by(from, 0, 1));
  h.moveTo(h.by(from, 0, 3));
  h.release(h.by(from, 0, 3));
  const fluir::pt::ParseTree after = h.tree();

  ASSERT_EQ(h.stack().count(), 1);
  EXPECT_NE(after, before);
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  h.stack().redo();
  EXPECT_EQ(h.tree(), after);
}

TEST(PortDrag, AGestureThatEndsWhereItStartedPushesNothing) {
  Harness h;
  const int start = h.y(kInput);
  const QPoint from = h.grab(kInput);

  h.press(from);
  h.moveTo(h.by(from, 0, 2));
  h.moveTo(from);
  h.release(from);

  EXPECT_EQ(h.y(kInput), start);
  EXPECT_EQ(h.stack().count(), 0);
}

TEST(PortDrag, EscapeRestoresThePort) {
  Harness h;
  const fluir::pt::ParseTree before = h.tree();
  const QPoint from = h.grab(kInput);
  h.press(from);
  h.moveTo(h.by(from, 0, 2));

  h.key(Qt::Key_Escape);
  EXPECT_EQ(h.tree(), before);
  h.release(h.by(from, 0, 2));

  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(h.stack().count(), 0);
}

TEST(PortDrag, AnOutputPortSlidesAlongTheRightWall) {
  Harness h;
  const int start = h.y(kOutput);
  const QPoint from = h.grab(kOutput);

  h.drag(from, h.by(from, 0, 2));

  EXPECT_EQ(h.y(kOutput), start + 2);
  EXPECT_DOUBLE_EQ(h.port(kOutput).sceneBoundingRect().center().x(), h.conditional().sceneBoundingRect().right());
}

TEST(PortDrag, ConduitsFollowAMovedPort) {
  Harness h;
  const QPoint input = h.grab(kInput);
  h.drag(input, h.by(input, 0, 4));
  EXPECT_TRUE(routed(h.find<diagram::Conduit>(8))) << "inside, from the input port";

  const QPoint output = h.grab(kOutput);
  h.drag(output, h.by(output, 0, 4));
  EXPECT_TRUE(routed(h.find<diagram::Conduit>(9))) << "inside, to the output port";
}
