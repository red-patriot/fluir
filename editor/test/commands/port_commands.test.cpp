#include <algorithm>
#include <stdexcept>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/commands/add_port.hpp"
#include "editor/commands/delete_port.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/ids.hpp"
#include "editor/diagram/port.hpp"
#include "editor/view/draw/conditional.hpp"
#include "interaction.hpp"

namespace {

  namespace diagram = fluir::editor::diagram;
  namespace draw = fluir::editor::draw;
  namespace pt = fluir::pt;
  using fluir::editor::AddPort;
  using fluir::editor::DeletePort;
  using testutil::Interaction;
  using testutil::routed;

  constexpr draw::PortRef kFirstInput{.output = false, .index = 1};
  constexpr draw::PortRef kSecondInput{.output = false, .index = 2};
  constexpr draw::PortRef kOutput{.output = true, .index = 0};

  pt::Block& bodyOf(pt::ParseTree& tree) { return std::get<pt::FunctionDecl>(tree.declarations.at(1)).body; }

  pt::Conditional& conditionalOf(pt::ParseTree& tree) { return std::get<pt::Conditional>(bodyOf(tree).nodes.at(2)); }

  struct Harness : Interaction {
    Harness() : Interaction("interaction/ported_conditional.fl") { }

    diagram::Conditional& conditional() { return find<diagram::Conditional>(2); }

    // Conduit `id` among the function body's children.
    diagram::Conduit& bodyConduit(fluir::ID id) {
      for (QGraphicsItem* child : find<diagram::Function>(1).body()->childItems()) {
        if (auto* conduit = dynamic_cast<diagram::Conduit*>(child); conduit != nullptr && conduit->id() == id) {
          return *conduit;
        }
      }
      throw std::logic_error("no such conduit");
    }

    void expectPortsInStep() {
      const draw::Ports& ports = conditional().ports();
      std::vector<draw::PortRef> refs;
      for (const diagram::Port* port : conditional().wallPorts()) {
        refs.push_back(port->ref());
        EXPECT_EQ(port->outer()->index(), static_cast<int>(port->ref().index));
        EXPECT_EQ(port->innerId(), draw::portOf(ports, port->ref())->innerId);
        EXPECT_DOUBLE_EQ(port->sceneBoundingRect().top(),
                         conditional().sceneBoundingRect().top() + draw::portOf(ports, port->ref())->y * unit());
      }
      EXPECT_EQ(refs, draw::portRefs(ports));
    }
  };

}  // namespace

TEST(AddPort, AppendsAnInputWithAFreshInnerIdAtTheWallsMiddleAndUndoRemovesIt) {
  Harness h;
  const pt::ParseTree before = h.tree();
  const int height = h.conditional().location().height;
  const fluir::editor::Limits<int> limits = draw::portYLimits(height, h.ctx().layout);
  pt::ParseTree expected = before;
  conditionalOf(expected).inputs.push_back({.innerId = diagram::nextId(*h.conditional().thenBranch()),
                                            .y = std::clamp(height / 2, limits.lower, limits.upper)});

  h.stack().push(new AddPort(h.conditional(), false));

  EXPECT_EQ(h.tree(), expected);
  h.expectPortsInStep();
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  h.expectPortsInStep();
  h.stack().redo();
  EXPECT_EQ(h.tree(), expected);
}

TEST(AddPort, AppendsAnOutputAfterTheExistingOnes) {
  Harness h;
  const fluir::ID innerId = diagram::nextId(*h.conditional().thenBranch());

  h.stack().push(new AddPort(h.conditional(), true));

  ASSERT_EQ(h.conditional().ports().outputs.size(), 2U);
  EXPECT_EQ(h.conditional().ports().outputs.back().innerId, innerId);
  h.expectPortsInStep();
}

TEST(DeletePort, DeletingAnInputShiftsLaterOuterWiresDownAndDropsItsInnerWires) {
  Harness h;
  const pt::ParseTree before = h.tree();
  pt::ParseTree expected = before;
  pt::Conditional& conditional = conditionalOf(expected);
  conditional.inputs.erase(conditional.inputs.begin());
  conditional.elseScope->conduits.erase(8);
  bodyOf(expected).conduits.at(6).children.front().index = 1;

  h.stack().push(new DeletePort(h.conditional(), kFirstInput));

  EXPECT_EQ(h.tree(), expected);
  h.expectPortsInStep();
  EXPECT_TRUE(routed(h.bodyConduit(6)));
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  h.expectPortsInStep();
  EXPECT_TRUE(routed(h.bodyConduit(6)));
  h.stack().redo();
  EXPECT_EQ(h.tree(), expected);
}

TEST(DeletePort, DeletingAWiredInputDropsItsWiresOnBothSides) {
  Harness h;
  const pt::ParseTree before = h.tree();
  pt::ParseTree expected = before;
  pt::Conditional& conditional = conditionalOf(expected);
  conditional.inputs.pop_back();
  conditional.thenScope->conduits.erase(6);
  bodyOf(expected).conduits.erase(6);

  h.stack().push(new DeletePort(h.conditional(), kSecondInput));

  EXPECT_EQ(h.tree(), expected);
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
}

TEST(DeletePort, DeletingAnOutputDropsItsWiresOutsideAndInBothBranches) {
  Harness h;
  const pt::ParseTree before = h.tree();
  pt::ParseTree expected = before;
  pt::Conditional& conditional = conditionalOf(expected);
  conditional.outputs.clear();
  conditional.thenScope->conduits.erase(7);
  conditional.elseScope->conduits.erase(8);
  bodyOf(expected).conduits.erase(7);

  h.stack().push(new DeletePort(h.conditional(), kOutput));

  EXPECT_EQ(h.tree(), expected);
  h.expectPortsInStep();
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  h.expectPortsInStep();
  EXPECT_TRUE(routed(h.bodyConduit(7)));
}
