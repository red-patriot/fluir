#include "editor/diagram/scope.hpp"

#include <filesystem>
#include <set>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/scope_path.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/node.hpp"
#include "interaction.hpp"

// A scope's path names, in the exported tree, the block holding exactly the scope's nodes.

namespace {

  namespace diagram = fluir::editor::diagram;
  namespace fs = std::filesystem;
  using fluir::editor::ELSE_BRANCH_ID;
  using fluir::editor::THEN_BRANCH_ID;
  using testutil::Interaction;

}  // namespace

TEST(PathOf, AFunctionsBodyIsItsId) {
  Interaction h("interaction/ported_conditional.fl");

  EXPECT_EQ(diagram::pathOf(*h.find<diagram::Function>(1).body()), (fluir::FullID{1}));
}

TEST(PathOf, ABranchFollowsItsConditional) {
  Interaction h("interaction/ported_conditional.fl");
  const auto& conditional = h.find<diagram::Conditional>(2);

  EXPECT_EQ(diagram::pathOf(*conditional.thenBranch()), (fluir::FullID{1, 2, THEN_BRANCH_ID}));
  EXPECT_EQ(diagram::pathOf(*conditional.elseBranch()), (fluir::FullID{1, 2, ELSE_BRANCH_ID}));
}

class PathOfOnEachFixture : public testing::TestWithParam<fs::path> { };

INSTANTIATE_TEST_SUITE_P(Fixtures,
                         PathOfOnEachFixture,
                         testing::ValuesIn(testutil::readWriteFixtures(true)),
                         testutil::fixtureName);

TEST_P(PathOfOnEachFixture, EachScopesPathNamesTheBlockOfItsNodes) {
  Interaction h(GetParam().string());
  const fluir::pt::ParseTree tree = h.tree();

  for (const diagram::Scope* scope : testutil::all<diagram::Scope>(h.scene())) {
    const fluir::pt::Block* block = fluir::editor::blockAt(tree, diagram::pathOf(*scope));
    ASSERT_NE(block, nullptr);
    std::set<fluir::ID> expected;
    for (const auto& [id, node] : block->nodes) {
      expected.insert(id);
    }
    std::set<fluir::ID> actual;
    for (const QGraphicsItem* child : scope->childItems()) {
      if (const auto* node = dynamic_cast<const diagram::FluirNode*>(child)) {
        actual.insert(node->id());
      }
    }
    EXPECT_EQ(actual, expected);
  }
}
