#include "editor/diagram/branch_arrow.hpp"

#include <gtest/gtest.h>

#include "editor/diagram/conditional.hpp"
#include "interaction.hpp"

// A press on a conditional's arrow shows its other branch. Which branch shows is view state: nothing is pushed.

namespace {

  namespace diagram = fluir::editor::diagram;
  using testutil::Interaction;

  void pressArrow(Interaction& h, const diagram::Conditional& conditional) {
    h.click(h.aim(Interaction::part<diagram::BranchArrow>(conditional)));
  }

}  // namespace

TEST(BranchArrow, APressShowsTheOtherBranch) {
  Interaction h("read/conditional_with_body.fl");
  auto& conditional = h.find<diagram::Conditional>(2);

  pressArrow(h, conditional);
  EXPECT_FALSE(conditional.thenBranch()->isVisible());
  EXPECT_TRUE(conditional.elseBranch()->isVisible());

  pressArrow(h, conditional);
  EXPECT_TRUE(conditional.thenBranch()->isVisible());
  EXPECT_FALSE(conditional.elseBranch()->isVisible());
}

TEST(BranchArrow, SwitchingPushesNothing) {
  Interaction h("read/conditional_with_body.fl");
  const fluir::pt::ParseTree before = h.tree();

  pressArrow(h, h.find<diagram::Conditional>(2));

  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(h.stack().count(), 0);
}

TEST(BranchArrow, APressSelectsItsConditional) {
  Interaction h("read/conditional_with_body.fl");
  auto& conditional = h.find<diagram::Conditional>(2);

  pressArrow(h, conditional);

  EXPECT_EQ(h.scene().selectedItems(), QList<QGraphicsItem*>{&conditional});
}

TEST(BranchArrow, EachConditionalSwitchesAlone) {
  Interaction h("interaction/two_conditionals.fl");
  auto& left = h.find<diagram::Conditional>(2);
  auto& right = h.find<diagram::Conditional>(3);

  pressArrow(h, left);

  EXPECT_TRUE(left.elseBranch()->isVisible());
  EXPECT_TRUE(right.thenBranch()->isVisible());
  EXPECT_FALSE(right.elseBranch()->isVisible());
}
