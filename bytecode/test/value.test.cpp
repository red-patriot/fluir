#include "bytecode/value.h"

#include <tuple>

#include <gtest/gtest.h>

using std::tuple;

TEST(TestValue, InitializeFromDouble) {
  double expected = 4.5;

  fluir::code::Value value = expected;

  EXPECT_DOUBLE_EQ(expected, value.asFloat64());
}

class TestValueRemembersType : public ::testing::TestWithParam<tuple<fluir::code::ValueType, fluir::code::Value>> { };

TEST_P(TestValueRemembersType, Test) {
  const auto& [type, value] = GetParam();

  EXPECT_EQ(type, value.type());
}

INSTANTIATE_TEST_SUITE_P(,
                         TestValueRemembersType,
                         ::testing::Values(tuple{fluir::code::ValueType::FLOAT64, fluir::code::Value{12.5}}));
