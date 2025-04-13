#include <gtest/gtest.h>

#include "fluir/compiler/afg.hpp"

namespace fafg = fluir::afg;

TEST(TestNode, MakeDoubleConstant) {
  auto actual = fafg::makeDoubleConstant(1.0);

  EXPECT_EQ(fluir::Type::DOUBLE_FP, actual.type());
  EXPECT_DOUBLE_EQ(1.0, actual.value);
}

TEST(TestNode, CompareSharedNodesDirectlyForEquality) {
  auto first = fafg::makeDependency<fafg::DoubleConstant>(3.5);
  auto second = fafg::makeDependency<fafg::DoubleConstant>(3.5);

  EXPECT_EQ(*first, *second);
}

TEST(TestNode, CompareSharedNodesDirectlyForEqualityWorksForDifferentTypes) {
  auto first = fafg::makeDoubleConstant(3.5);
  auto second = fafg::BinaryOp{
      fluir::Operator::STAR,
      fafg::makeDependency<fafg::DoubleConstant>(1.0),
      fafg::makeDependency<fafg::DoubleConstant>(2.0)};

  EXPECT_NE(first, second);
}

TEST(TestNode, ConstructBinaryOperation) {
  auto expected = fafg::BinaryOp{
      fluir::Operator::PLUS,
      fafg::makeDependency<fafg::DoubleConstant>(1.0),
      fafg::makeDependency<fafg::DoubleConstant>(2.0)};

  auto actual = fafg::BinaryOp{
      fluir::Operator::PLUS,
      fafg::makeDependency<fafg::DoubleConstant>(1.0),
      fafg::makeDependency<fafg::DoubleConstant>(2.0)};

  EXPECT_EQ(expected, actual);
}

TEST(TestNode, ConstructUnaryOperation) {
  auto expected = fafg::UnaryOp{
      fluir::Operator::MINUS,
      fafg::makeDependency<fafg::DoubleConstant>(7.5)};

  auto actual = fafg::UnaryOp{
      fluir::Operator::MINUS,
      fafg::makeDependency<fafg::DoubleConstant>(7.5)};

  EXPECT_EQ(expected, actual);
}
