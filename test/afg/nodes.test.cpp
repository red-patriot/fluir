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

TEST(TestNode, CompareSharedNodesDirectlyForEqualityWorksForDifferentTypes) {
  auto first = fafg::shared(fafg::makeDoubleConstant(3.5));
  auto second = fafg::shared(fafg::BinaryOp{
      fluir::Operator::STAR,
      fafg::shared(fafg::makeDoubleConstant(1.0)),
      fafg::shared(fafg::makeDoubleConstant(2.0))});

  EXPECT_NE(*first, *second);
}

TEST(TestNode, ConstructBinaryOperation) {
  auto expected = fafg::BinaryOp{
      fluir::Operator::PLUS,
      fafg::shared(fafg::makeDoubleConstant(1.0)),
      fafg::shared(fafg::makeDoubleConstant(2.0))};

  auto actual = fafg::shared(fafg::BinaryOp{
      fluir::Operator::PLUS,
      fafg::shared(fafg::makeDoubleConstant(1.0)),
      fafg::shared(fafg::makeDoubleConstant(2.0))});

  EXPECT_EQ(expected, *actual);
}

TEST(TestNode, ConstructUnaryOperation) {
  auto expected = fafg::UnaryOp{
      fluir::Operator::MINUS,
      fafg::shared(fafg::makeDoubleConstant(7.5))};

  auto actual = fafg::shared(fafg::UnaryOp{
      fluir::Operator::MINUS,
      fafg::shared(fafg::makeDoubleConstant(7.5))});

  EXPECT_EQ(expected, *actual);
}
