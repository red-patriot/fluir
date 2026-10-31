#include "editor/diagram/ids.hpp"

#include <gtest/gtest.h>

#include "editor/diagram/conditional.hpp"
#include "editor/diagram/function.hpp"
#include "interaction.hpp"

namespace {

  namespace diagram = fluir::editor::diagram;
  using diagram::nextId;
  using testutil::Interaction;

}  // namespace

TEST(NextId, AnEmptyBodyGetsAValidId) {
  Interaction h{"read/single_empty_function.fl"};

  EXPECT_NE(nextId(*h.find<diagram::Function>(1).body()), fluir::INVALID_ID);
}

TEST(NextId, ABodyExceedsEveryNodeConduitParameterAndReturn) {
  Interaction h{"interaction/wired_function.fl"};

  EXPECT_EQ(nextId(*h.find<diagram::Function>(1).body()), 10);
}

TEST(NextId, ABranchExceedsItsConditionalsPortInnerIds) {
  Interaction h{"write/conditional_with_port.fl"};
  auto& conditional = h.find<diagram::Conditional>(2);

  EXPECT_EQ(nextId(*conditional.thenBranch()), 22);
  EXPECT_EQ(nextId(*conditional.elseBranch()), 22);
}

TEST(NextId, ABranchExceedsIdsInItsSiblingBranch) {
  Interaction h{"read/conditional_with_body.fl"};
  auto& conditional = h.find<diagram::Conditional>(2);

  EXPECT_EQ(nextId(*conditional.elseBranch()), 17);
}
