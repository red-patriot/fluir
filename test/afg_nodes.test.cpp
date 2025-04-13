#include <gtest/gtest.h>

#include "fluir/compiler/afg.hpp"

namespace fafg = fluir::afg;

TEST(TestNode, MakeDoubleConstant) {
  auto actual = fafg::makeDoubleConstant(1.0);

  EXPECT_EQ(fluir::Type::DOUBLE_FP, actual.type());
  EXPECT_DOUBLE_EQ(1.0, actual.as<fluir::Type::DOUBLE_FP>());
}

TEST(TestNode, MakeShared) {
  auto expected = fafg::makeDoubleConstant(3.5);

  auto actual = fafg::shared(fafg::makeDoubleConstant(3.5));

  EXPECT_EQ(expected.type(), actual->type());
  EXPECT_TRUE(actual->as<fafg::Constant>());
  EXPECT_DOUBLE_EQ(expected.as<fluir::Type::DOUBLE_FP>(),
                   actual->as<fafg::Constant>()->as<fluir::Type::DOUBLE_FP>());
}

TEST(TestNode, CompareSharedNodesDirectlyForEquality) {
  auto first = fafg::shared(fafg::makeDoubleConstant(3.5));
  auto second = fafg::shared(fafg::makeDoubleConstant(3.5));

  EXPECT_EQ(*first, *second);
}
