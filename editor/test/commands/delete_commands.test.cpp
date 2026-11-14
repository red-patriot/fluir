#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/commands/delete_conduit.hpp"
#include "editor/commands/delete_node.hpp"
#include "editor/diagram/binary.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/function.hpp"
#include "interaction.hpp"

// Each delete removes what it names; undo brings back the same items, wired as before, and redo removes them again.
// fan_out.fl: constant 1 feeds binary 2 and unary 3 through conduit 4; constant 5 feeds binary 2 through conduit 6.

namespace {

  namespace diagram = fluir::editor::diagram;
  namespace pt = fluir::pt;
  using fluir::editor::DeleteConduit;
  using fluir::editor::DeleteNode;
  using testutil::Interaction;
  using testutil::routed;

  pt::Block& bodyOf(pt::ParseTree& tree) { return std::get<pt::FunctionDecl>(tree.declarations.at(1)).body; }

  void expectAllRouted(Interaction& h) {
    for (const diagram::Conduit* conduit : testutil::all<diagram::Conduit>(h.scene())) {
      EXPECT_TRUE(routed(*conduit)) << "conduit " << conduit->id();
    }
  }

}  // namespace

TEST(DeleteNode, RemovesTheNodeAndCutsItsConduits) {
  Interaction h("interaction/fan_out.fl");
  pt::ParseTree expected = h.tree();
  bodyOf(expected).nodes.erase(2);
  bodyOf(expected).conduits.erase(6);
  std::erase_if(bodyOf(expected).conduits.at(4).children, [](const auto& sink) { return sink.target == 2; });

  h.stack().push(new DeleteNode(h.find<diagram::Binary>(2)));

  EXPECT_EQ(h.tree(), expected);
  expectAllRouted(h);
}

TEST(DeleteNode, UndoBringsBackTheSameItemsWiredAsBefore) {
  Interaction h("interaction/fan_out.fl");
  const pt::ParseTree before = h.tree();
  auto& binary = h.find<diagram::Binary>(2);
  auto& cut = h.find<diagram::Conduit>(6);
  h.stack().push(new DeleteNode(binary));
  const pt::ParseTree after = h.tree();
  ASSERT_NE(after, before);

  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(&h.find<diagram::Binary>(2), &binary);
  EXPECT_EQ(&h.find<diagram::Conduit>(6), &cut);
  expectAllRouted(h);

  h.stack().redo();
  EXPECT_EQ(h.tree(), after);
}

TEST(DeleteNode, ATopLevelNodeGoesAndComesBack) {
  Interaction h("read/multiple_empty_functions.fl");
  const pt::ParseTree before = h.tree();
  pt::ParseTree expected = before;
  expected.declarations.erase(7);
  auto& function = h.find<diagram::Function>(7);

  h.stack().push(new DeleteNode(function));
  EXPECT_EQ(h.tree(), expected);

  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(&h.find<diagram::Function>(7), &function);
}

TEST(DeleteConduit, RemovesOnlyItAndUndoReattachesIt) {
  Interaction h("interaction/fan_out.fl");
  const pt::ParseTree before = h.tree();
  pt::ParseTree expected = before;
  bodyOf(expected).conduits.erase(4);
  auto& conduit = h.find<diagram::Conduit>(4);

  h.stack().push(new DeleteConduit(conduit));
  EXPECT_EQ(h.tree(), expected);

  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(&h.find<diagram::Conduit>(4), &conduit);
  expectAllRouted(h);

  h.stack().redo();
  EXPECT_EQ(h.tree(), expected);
}
