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

TEST(TestValue, InitializeInt32) {
  std::int32_t expected = 190;

  fluir::code::Value value{expected};

  ASSERT_EQ(fluir::code::ValueType::I32, value.type());
  EXPECT_DOUBLE_EQ(expected, value.asI32());
}

TEST(TestValue, InitializeInt16) {
  std::int16_t expected = 13;

  fluir::code::Value value{expected};

  ASSERT_EQ(fluir::code::ValueType::I16, value.type());
  EXPECT_DOUBLE_EQ(expected, value.asI16());
}
TEST(TestValue, InitializeInt8) {
  std::int8_t expected = -4;

  fluir::code::Value value{expected};

  ASSERT_EQ(fluir::code::ValueType::I8, value.type());
  EXPECT_DOUBLE_EQ(expected, value.asI8());
}

TEST(TestValue, InitializeUint64) {
  std::uint64_t expected = 12345;

  fluir::code::Value value{expected};

  ASSERT_EQ(fluir::code::ValueType::U64, value.type());
  EXPECT_DOUBLE_EQ(expected, value.asU64());
}

TEST(TestValue, InitializeUint32) {
  std::uint32_t expected = 321;

  fluir::code::Value value{expected};

  ASSERT_EQ(fluir::code::ValueType::U32, value.type());
  EXPECT_DOUBLE_EQ(expected, value.asU32());
}

TEST(TestValue, InitializeUint16) {
  std::uint16_t expected = 12;

  fluir::code::Value value{expected};

  ASSERT_EQ(fluir::code::ValueType::U16, value.type());
  EXPECT_DOUBLE_EQ(expected, value.asU16());
}

TEST(TestValue, InitializeUint8) {
  std::uint8_t expected = 89;

  fluir::code::Value value{expected};

  ASSERT_EQ(fluir::code::ValueType::U8, value.type());
  EXPECT_DOUBLE_EQ(expected, value.asU8());
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
