#include "fluir/compiler/afg/dependency.hpp"

#include <gtest/gtest.h>

#include "fluir/compiler/afg.hpp"

namespace fafg = fluir::afg;

TEST(TestDependency, DereferenceToGetSourceNode) {
  auto expected = fafg::makeDoubleConstant(4.0);

  auto actual = fafg::makeDependency(fafg::makeDoubleConstant(4.0));

  EXPECT_TRUE(expected.equals(*actual));
}

TEST(TestDependency, DereferenceToGetSourceNodeConst) {
  auto expected = fafg::makeDoubleConstant(4.0);

  const auto actual = fafg::makeDependency(fafg::makeDoubleConstant(4.0));

  EXPECT_TRUE(expected.equals(*actual));
}

TEST(TestDependency, ArrowOperatorWorksAsExpected) {
  auto expected = fafg::makeDoubleConstant(3.5);

  auto actual = fafg::makeDependency(fafg::makeDoubleConstant(3.5));

  EXPECT_TRUE(actual->equals(expected));
}

TEST(TestDependency, ArrowOperatorWorksAsExpectedWhenConst) {
  auto expected = fafg::makeDoubleConstant(3.5);

  const auto actual = fafg::makeDependency(fafg::makeDoubleConstant(3.5));

  EXPECT_TRUE(actual->equals(expected));
}

TEST(TestDependency, CanGetUnderlyingPointer) {
  auto expected = fafg::makeDoubleConstant(0.5);

  auto actual = fafg::makeDependency(fafg::makeDoubleConstant(0.5));

  EXPECT_TRUE(actual.get()->equals(expected));
}

TEST(TestDependency, CanGetUnderlyingConstPointer) {
  auto expected = fafg::makeDoubleConstant(0.5);

  const auto actual = fafg::makeDependency(fafg::makeDoubleConstant(0.5));

  EXPECT_TRUE(actual.get()->equals(expected));
}

TEST(TestDependency, CopyConstructingSharesDependency) {
  auto first = fafg::makeDependency(fafg::makeDoubleConstant(-12.432));
  auto second = first;

  EXPECT_EQ(first.get(), second.get());
}

TEST(TestDependency, CopyAssigningSharesDependency) {
  auto first = fafg::makeDependency(fafg::UnaryOp{
      fluir::Operator::PLUS,
      fafg::makeDependency(fafg::makeDoubleConstant(10.6))});
  auto second = fafg::makeDependency(fafg::makeDoubleConstant(12345.0));

  EXPECT_NE(first.get(), second.get());

  second = first;

  EXPECT_EQ(first.get(), second.get());
}

TEST(TestDependency, MoveConstructingMakesACopy) {
  auto first = fafg::makeDependency(fafg::makeDoubleConstant(-12.432));
  auto second = std::move(first);

  EXPECT_EQ(first.get(), second.get());
}

TEST(TestDependency, MoveAssigningMakesACopy) {
  auto first = fafg::makeDependency(fafg::makeDoubleConstant(6.5467));
  auto second = fafg::makeDependency(fafg::makeDoubleConstant(12345.0));

  EXPECT_NE(first.get(), second.get());

  second = std::move(first);

  EXPECT_EQ(first.get(), second.get());
}

TEST(TestDependency, GetUpstreamCount) {
  auto first = fafg::makeDependency(fafg::makeDoubleConstant(-12.432));

  EXPECT_EQ(1, first.upstreamCount());

  auto second = first;

  EXPECT_EQ(2, first.upstreamCount());
  EXPECT_EQ(2, second.upstreamCount());
}

TEST(TestDependency, IndicatesIfUnique) {
  auto first = fafg::makeDependency(fafg::makeDoubleConstant(-12.432));

  EXPECT_TRUE(first.unique());

  auto second = first;

  EXPECT_FALSE(first.unique());
  EXPECT_FALSE(second.unique());
}

TEST(TestDependency, CanReswingAllCommonDependers) {
  auto first = fafg::makeDependency(fafg::makeDoubleConstant(-12.432));
  auto second = first;

  EXPECT_FALSE(dynamic_cast<fafg::UnaryOp*>(first.get()));
  EXPECT_FALSE(dynamic_cast<fafg::UnaryOp*>(second.get()));

  first.reset<fafg::UnaryOp>(
      fluir::Operator::MINUS,
      fafg::makeDependency(fafg::makeDoubleConstant(7.5)));

  EXPECT_EQ(first.get(), second.get());
  EXPECT_TRUE(dynamic_cast<fafg::UnaryOp*>(second.get()));
}
