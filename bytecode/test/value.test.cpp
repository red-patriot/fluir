#include "bytecode/value.h"

#include <cstdint>
#include <tuple>

#include <gtest/gtest.h>

using std::tuple;

using namespace fluir::code::value_literals;

TEST(TestValue, InitializeFloat64) {
  double expected = 4.5;

  fluir::code::Value value{expected};

  ASSERT_EQ(fluir::code::ValueType::F64, value.type());
  EXPECT_DOUBLE_EQ(expected, value.asF64());
}

TEST(TestValue, InitializeInt64) {
  std::int64_t expected = 31;

  fluir::code::Value value{expected};

  ASSERT_EQ(fluir::code::ValueType::I64, value.type());
  EXPECT_DOUBLE_EQ(expected, value.asI64());
}

TEST(TestValue, InitializeUint64) {
  std::uint64_t expected = 12345;

  fluir::code::Value value{expected};

  ASSERT_EQ(fluir::code::ValueType::U64, value.type());
  EXPECT_DOUBLE_EQ(expected, value.asU64());
}

TEST(TestValue, I64AccessChecksForFloat) {
  fluir::code::Value value = 4.5_f64;

  EXPECT_THROW(auto _ = value.asI64(), std::runtime_error);
}

TEST(TestValue, F64AccessChecksForFloat) {
  fluir::code::Value value = 12_u64;

  EXPECT_THROW(auto _ = value.asF64(), std::runtime_error);
}

TEST(TestValue, U64AccessChecksForFloat) {
  fluir::code::Value value = 12_i64;

  EXPECT_THROW(auto _ = value.asU64(), std::runtime_error);
}
