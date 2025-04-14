#include <gtest/gtest.h>

#include "fluir/compiler/afg.hpp"

namespace fafg = fluir::afg;

TEST(TestFlowGraph, AddAndAccessNodesByIndex) {
  fluir::afg::FlowGraph actual{};
  actual.add(std::make_unique<fafg::BinaryOp>(
      fluir::Operator::PLUS,
      fafg::makeDependency<fafg::DoubleConstant>(1.0),
      fafg::makeDependency<fafg::DoubleConstant>(2.0)));
  actual.add(std::make_unique<fafg::UnaryOp>(
      fluir::Operator::MINUS,
      fafg::makeDependency<fafg::DoubleConstant>(7.5)));

  auto expected0 = fafg::BinaryOp(
      fluir::Operator::PLUS,
      fafg::makeDependency<fafg::DoubleConstant>(1.0),
      fafg::makeDependency<fafg::DoubleConstant>(2.0));
  auto expected1 = fafg::UnaryOp(
      fluir::Operator::MINUS,
      fafg::makeDependency<fafg::DoubleConstant>(7.5));

  EXPECT_EQ(expected0, actual.at(0));
  EXPECT_EQ(expected1, actual.at(1));
}
