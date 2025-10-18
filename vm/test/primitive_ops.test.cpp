#include <cmath>
#include <tuple>

#include <gtest/gtest.h>

#include "vm/vm.hpp"

using std::tuple;

namespace fc = fluir::code;
using enum fluir::code::Instruction;
using namespace fluir::code::value_literals;

class TestPrimitiveOps : public ::testing::TestWithParam<tuple<fc::Value, fc::Chunk>> { };

TEST_P(TestPrimitiveOps, Test) {
  auto [expected, chunk] = GetParam();
  // Add postamble to bytecode to make it execute correctly without requiring too much boilerplate in tests
  chunk.code.push_back(EXIT);
  fc::ByteCode code{.header = {}, .chunks = {std::move(chunk)}};

  fluir::VirtualMachine vm;
  EXPECT_EQ(fluir::ExecResult::SUCCESS, vm.execute(&code));
  auto actual = vm.viewStack().back();
  if (actual.type() == fc::PrimitiveType::F64) {
    // stupid double epsilon issues
    EXPECT_DOUBLE_EQ(expected.asF64(), actual.asF64());
  } else {
    EXPECT_EQ(expected, actual);
  }
}

INSTANTIATE_TEST_SUITE_P(
  Float,
  TestPrimitiveOps,
  ::testing::Values(
    // F64 basic ops
    tuple{3.5_f64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_ADD}, .constants = {1.5_f64, 2.0_f64}}},
    tuple{1.25_f64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_SUB}, .constants = {1.5_f64, 0.25_f64}}},
    tuple{12.0_f64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_MUL}, .constants = {1.5_f64, 8.0_f64}}},
    tuple{0.75_f64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_DIV}, .constants = {3.0_f64, 4.0_f64}}},
    // F64 edge cases
    tuple{fc::Value{INFINITY}, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_MUL}, .constants = {1.0e200_f64, 2.0e200_f64}}},
    tuple{fc::Value{INFINITY}, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_DIV}, .constants = {3.0_f64, 0.0_f64}}},
    tuple{fc::Value{-INFINITY},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_DIV}, .constants = {fc::Value{-3.0}, 0.0_f64}}},
    tuple{fc::Value{INFINITY}, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_ADD}, .constants = {1.0e308_f64, 1.0e308_f64}}},
    tuple{fc::Value{-INFINITY},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_SUB}, .constants = {fc::Value{-1.0e308}, 1.0e308_f64}}},
    tuple{0.0_f64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_MUL}, .constants = {0.0_f64, 1000.0_f64}}},
    tuple{fc::Value{-5.5},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_ADD}, .constants = {fc::Value{-2.5}, fc::Value{-3.0}}}},
    tuple{2.5_f64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_SUB}, .constants = {fc::Value{-2.5}, fc::Value{-5.0}}}},
    tuple{15.0_f64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_MUL}, .constants = {fc::Value{-3.0}, fc::Value{-5.0}}}},
    tuple{fc::Value{-2.0}, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_DIV}, .constants = {10.0_f64, fc::Value{-5.0}}}}));

INSTANTIATE_TEST_SUITE_P(
  I8,
  TestPrimitiveOps,
  ::testing::Values(
    tuple{5_i8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD}, .constants = {2_i8, 3_i8}}},
    tuple{10_i8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB}, .constants = {15_i8, 5_i8}}},
    tuple{20_i8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL}, .constants = {4_i8, 5_i8}}},
    tuple{3_i8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {15_i8, 5_i8}}},
    tuple{fc::Value{static_cast<int8_t>(-5)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD},
                    .constants = {fc::Value{static_cast<int8_t>(-2)}, fc::Value{static_cast<int8_t>(-3)}}}},
    tuple{5_i8,
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD}, .constants = {10_i8, fc::Value{static_cast<int8_t>(-5)}}}},
    tuple{fc::Value{(int8_t)-15},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB}, .constants = {fc::Value{static_cast<int8_t>(-10)}, 5_i8}}},
    tuple{fc::Value{(int8_t)-20},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL}, .constants = {fc::Value{static_cast<int8_t>(-4)}, 5_i8}}},
    tuple{20_i8,
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL},
                    .constants = {fc::Value{static_cast<int8_t>(-4)}, fc::Value{static_cast<int8_t>(-5)}}}},
    tuple{fc::Value{(int8_t)-3},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {15_i8, fc::Value{static_cast<int8_t>(-5)}}}},
    tuple{fc::Value{(int8_t)-3},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {fc::Value{static_cast<int8_t>(-15)}, 5_i8}}},
    tuple{3_i8,
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV},
                    .constants = {fc::Value{static_cast<int8_t>(-15)}, fc::Value{static_cast<int8_t>(-5)}}}},
    tuple{static_cast<int8_t>(-128), fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD}, .constants = {127_i8, 1_i8}}},
    tuple{127_i8,
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB}, .constants = {fc::Value{static_cast<int8_t>(-128)}, 1_i8}}},
    tuple{fc::Value{static_cast<int8_t>(-2)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL}, .constants = {127_i8, 2_i8}}}));
INSTANTIATE_TEST_SUITE_P(
  I16,
  TestPrimitiveOps,
  ::testing::Values(
    tuple{500_i16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD}, .constants = {200_i16, 300_i16}}},
    tuple{1000_i16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB}, .constants = {1500_i16, 500_i16}}},
    tuple{2000_i16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL}, .constants = {40_i16, 50_i16}}},
    tuple{30_i16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {150_i16, 5_i16}}},
    tuple{fc::Value{static_cast<int16_t>(-500)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD},
                    .constants = {fc::Value{static_cast<int16_t>(-200)}, fc::Value{static_cast<int16_t>(-300)}}}},
    tuple{
      500_i16,
      fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD}, .constants = {1000_i16, fc::Value{static_cast<int16_t>(-500)}}}},
    tuple{
      fc::Value{static_cast<int16_t>(-1500)},
      fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB}, .constants = {fc::Value{static_cast<int16_t>(-1000)}, 500_i16}}},
    tuple{fc::Value{static_cast<int16_t>(-2000)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL}, .constants = {fc::Value{static_cast<int16_t>(-40)}, 50_i16}}},
    tuple{2000_i16,
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL},
                    .constants = {fc::Value{static_cast<int16_t>(-40)}, fc::Value{static_cast<int16_t>(-50)}}}},
    tuple{fc::Value{static_cast<int16_t>(-30)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {150_i16, fc::Value{static_cast<int16_t>(-5)}}}},
    tuple{static_cast<int16_t>(-32768),
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD}, .constants = {32767_i16, 1_i16}}},
    tuple{
      32767_i16,
      fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB}, .constants = {fc::Value{static_cast<int16_t>(-32768)}, 1_i16}}},
    tuple{fc::Value{static_cast<int16_t>(-2)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL}, .constants = {32767_i16, 2_i16}}}));
INSTANTIATE_TEST_SUITE_P(
  I32,
  TestPrimitiveOps,
  ::testing::Values(
    tuple{50000_i32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD}, .constants = {20000_i32, 30000_i32}}},
    tuple{100000_i32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB}, .constants = {150000_i32, 50000_i32}}},
    tuple{200000_i32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL}, .constants = {4000_i32, 50_i32}}},
    tuple{3000_i32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {15000_i32, 5_i32}}},
    tuple{fc::Value{static_cast<int32_t>(-50000)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD},
                    .constants = {fc::Value{static_cast<int32_t>(-20000)}, fc::Value{static_cast<int32_t>(-30000)}}}},
    tuple{50000_i32,
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD},
                    .constants = {100000_i32, fc::Value{static_cast<int32_t>(-50000)}}}},
    tuple{fc::Value{static_cast<int32_t>(-150000)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB},
                    .constants = {fc::Value{static_cast<int32_t>(-100000)}, 50000_i32}}},
    tuple{
      fc::Value{static_cast<int32_t>(-200000)},
      fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL}, .constants = {fc::Value{static_cast<int32_t>(-4000)}, 50_i32}}},
    tuple{200000_i32,
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL},
                    .constants = {fc::Value{static_cast<int32_t>(-4000)}, fc::Value{static_cast<int32_t>(-50)}}}},
    tuple{
      fc::Value{static_cast<int32_t>(-3000)},
      fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {15000_i32, fc::Value{static_cast<int32_t>(-5)}}}},
    tuple{static_cast<int32_t>(-2147483648),
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD}, .constants = {2147483647_i32, 1_i32}}},
    tuple{2147483647_i32,
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB},
                    .constants = {fc::Value{static_cast<int32_t>(-2147483648)}, 1_i32}}},
    tuple{fc::Value{static_cast<int32_t>(-2)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL}, .constants = {2147483647_i32, 2_i32}}}));
INSTANTIATE_TEST_SUITE_P(
  I64,
  TestPrimitiveOps,
  ::testing::Values(
    tuple{5000000000_i64,
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD}, .constants = {2000000000_i64, 3000000000_i64}}},
    tuple{10000000000_i64,
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB}, .constants = {15000000000_i64, 5000000000_i64}}},
    tuple{20000000000_i64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL}, .constants = {4000000_i64, 5000_i64}}},
    tuple{3000000_i64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {15000000_i64, 5_i64}}},
    tuple{fc::Value{static_cast<int64_t>(-5000000000)},
          fc::Chunk{
            .code = {PUSH, 0, PUSH, 1, I64_ADD},
            .constants = {fc::Value{static_cast<int64_t>(-2000000000)}, fc::Value{static_cast<int64_t>(-3000000000)}}}},
    tuple{5000000000_i64,
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD},
                    .constants = {10000000000_i64, fc::Value{static_cast<int64_t>(-5000000000)}}}},
    tuple{fc::Value{static_cast<int64_t>(-15000000000)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB},
                    .constants = {fc::Value{static_cast<int64_t>(-10000000000)}, 5000000000_i64}}},
    tuple{fc::Value{static_cast<int64_t>(-20000000000)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL},
                    .constants = {fc::Value{static_cast<int64_t>(-4000000)}, 5000_i64}}},
    tuple{20000000000_i64,
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL},
                    .constants = {fc::Value{static_cast<int64_t>(-4000000)}, fc::Value{static_cast<int64_t>(-5000)}}}},
    tuple{
      fc::Value{static_cast<int64_t>(-3000000)},
      fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {15000000_i64, fc::Value{static_cast<int64_t>(-5)}}}},
    tuple{static_cast<int64_t>(-9223372036854775807LL - 1),
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD}, .constants = {9223372036854775807_i64, 1_i64}}},
    tuple{9223372036854775807_i64,
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB},
                    .constants = {fc::Value{static_cast<int64_t>(-9223372036854775807LL - 1)}, 1_i64}}},
    tuple{fc::Value{static_cast<int64_t>(-2)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL}, .constants = {9223372036854775807_i64, 2_i64}}}));

INSTANTIATE_TEST_SUITE_P(
  U8,
  TestPrimitiveOps,
  ::testing::Values(tuple{5_u8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {2_u8, 3_u8}}},
                    tuple{10_u8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {15_u8, 5_u8}}},
                    tuple{20_u8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {4_u8, 5_u8}}},
                    tuple{3_u8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {15_u8, 5_u8}}},
                    tuple{100_u8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {100_u8, 0_u8}}},
                    tuple{50_u8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {50_u8, 0_u8}}},
                    tuple{0_u8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {100_u8, 0_u8}}},
                    tuple{0_u8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {0_u8, 100_u8}}},
                    tuple{3_u8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {10_u8, 3_u8}}},
                    tuple{5_u8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {17_u8, 3_u8}}},
                    tuple{0_u8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {255_u8, 1_u8}}},
                    tuple{1_u8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {255_u8, 2_u8}}},
                    tuple{10_u8, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {255_u8, 11_u8}}},
                    tuple{fc::Value{static_cast<uint8_t>(254)},
                          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {255_u8, 2_u8}}},
                    tuple{fc::Value{static_cast<uint8_t>(255)},
                          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {0_u8, 1_u8}}},
                    tuple{fc::Value{static_cast<uint8_t>(251)},
                          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {5_u8, 10_u8}}},
                    tuple{fc::Value{static_cast<uint8_t>(245)},
                          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {0_u8, 11_u8}}}));
INSTANTIATE_TEST_SUITE_P(
  U16,
  TestPrimitiveOps,
  ::testing::Values(tuple{500_u16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {200_u16, 300_u16}}},
                    tuple{1000_u16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {1500_u16, 500_u16}}},
                    tuple{2000_u16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {40_u16, 50_u16}}},
                    tuple{30_u16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {150_u16, 5_u16}}},
                    tuple{10000_u16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {10000_u16, 0_u16}}},
                    tuple{5000_u16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {5000_u16, 0_u16}}},
                    tuple{0_u16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {10000_u16, 0_u16}}},
                    tuple{0_u16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {0_u16, 10000_u16}}},
                    tuple{333_u16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {1000_u16, 3_u16}}},
                    tuple{566_u16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {1700_u16, 3_u16}}},
                    tuple{0_u16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {65535_u16, 1_u16}}},
                    tuple{10_u16, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {65535_u16, 11_u16}}},
                    tuple{fc::Value{static_cast<uint16_t>(65534)},
                          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {65535_u16, 2_u16}}},
                    tuple{fc::Value{static_cast<uint16_t>(65535)},
                          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {0_u16, 1_u16}}},
                    tuple{fc::Value{static_cast<uint16_t>(65531)},
                          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {5_u16, 10_u16}}},
                    tuple{fc::Value{static_cast<uint16_t>(65525)},
                          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {0_u16, 11_u16}}}));
INSTANTIATE_TEST_SUITE_P(
  U32,
  TestPrimitiveOps,
  ::testing::Values(
    tuple{50000_u32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {20000_u32, 30000_u32}}},
    tuple{100000_u32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {150000_u32, 50000_u32}}},
    tuple{200000_u32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {4000_u32, 50_u32}}},
    tuple{3000_u32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {15000_u32, 5_u32}}},
    tuple{1000000_u32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {1000000_u32, 0_u32}}},
    tuple{500000_u32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {500000_u32, 0_u32}}},
    tuple{0_u32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {1000000_u32, 0_u32}}},
    tuple{0_u32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {0_u32, 1000000_u32}}},
    tuple{33333_u32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {100000_u32, 3_u32}}},
    tuple{56666_u32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {170000_u32, 3_u32}}},
    tuple{0_u32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {4294967295_u32, 1_u32}}},
    tuple{10_u32, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {4294967295_u32, 11_u32}}},
    tuple{fc::Value{static_cast<uint32_t>(4294967294)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {4294967295_u32, 2_u32}}},
    tuple{fc::Value{static_cast<uint32_t>(4294967295)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {0_u32, 1_u32}}},
    tuple{fc::Value{static_cast<uint32_t>(4294967291)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {5_u32, 10_u32}}},
    tuple{fc::Value{static_cast<uint32_t>(4294967285)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {0_u32, 11_u32}}}));
INSTANTIATE_TEST_SUITE_P(
  U64,
  TestPrimitiveOps,
  ::testing::Values(
    tuple{5000000000_u64,
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {2000000000_u64, 3000000000_u64}}},
    tuple{10000000000_u64,
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {15000000000_u64, 5000000000_u64}}},
    tuple{20000000000_u64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {4000000_u64, 5000_u64}}},
    tuple{3000000_u64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {15000000_u64, 5_u64}}},
    tuple{10000000000_u64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {10000000000_u64, 0_u64}}},
    tuple{5000000000_u64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {5000000000_u64, 0_u64}}},
    tuple{0_u64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {10000000000_u64, 0_u64}}},
    tuple{0_u64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {0_u64, 10000000000_u64}}},
    tuple{3333333333_u64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {10000000000_u64, 3_u64}}},
    tuple{5666666666_u64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {17000000000_u64, 3_u64}}},
    tuple{0_u64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {18446744073709551615_u64, 1_u64}}},
    tuple{10_u64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD}, .constants = {18446744073709551615_u64, 11_u64}}},
    tuple{fc::Value{static_cast<uint64_t>(18446744073709551614ULL)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL}, .constants = {18446744073709551615_u64, 2_u64}}},
    tuple{fc::Value{static_cast<uint64_t>(18446744073709551615ULL)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {0_u64, 1_u64}}},
    tuple{fc::Value{static_cast<uint64_t>(18446744073709551611ULL)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {5_u64, 10_u64}}},
    tuple{fc::Value{static_cast<uint64_t>(18446744073709551605ULL)},
          fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB}, .constants = {0_u64, 11_u64}}}));

// Test this separately since NAN isn't equal to NAN
class TestPrimitiveOpsProducingNans : public ::testing::TestWithParam<fc::Chunk> { };
TEST_P(TestPrimitiveOpsProducingNans, Test0Div) {
  auto chunk = GetParam();
  chunk.code.push_back(EXIT);
  // With NANs, compare slightly differently
  fc::ByteCode code{.header = {}, .chunks = {std::move(chunk)}};

  fluir::VirtualMachine vm;
  EXPECT_EQ(fluir::ExecResult::SUCCESS, vm.execute(&code));
  auto actual = vm.viewStack().back();
  EXPECT_TRUE(std::isnan(actual.asF64()));
}

static const auto NAN_FL_VALUE = fc::Value{NAN};

INSTANTIATE_TEST_SUITE_P(
  TestPrimitiveOpsProducingNans,
  TestPrimitiveOpsProducingNans,
  ::testing::Values(fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_DIV}, .constants = {0.0_f64, 0.0_f64}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_ADD}, .constants = {1.0_f64, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_SUB}, .constants = {2.0_f64, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_MUL}, .constants = {5.0_f64, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_DIV}, .constants = {3.0_f64, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_ADD}, .constants = {NAN_FL_VALUE, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_SUB}, .constants = {NAN_FL_VALUE, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_MUL}, .constants = {NAN_FL_VALUE, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_DIV}, .constants = {NAN_FL_VALUE, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, F64_NEG}, .constants = {NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, F64_AFF}, .constants = {NAN_FL_VALUE}}));

class TestDivideByZeroCrashes : public ::testing::TestWithParam<fc::Chunk> { };
TEST_P(TestDivideByZeroCrashes, Test) {
  auto chunk = GetParam();
  chunk.code.push_back(EXIT);
  // With NANs, compare slightly differently
  fc::ByteCode code{.header = {}, .chunks = {std::move(chunk)}};

  fluir::VirtualMachine vm;
  EXPECT_EQ(fluir::ExecResult::ERROR_DIVIDE_BY_ZERO, vm.execute(&code));
}

INSTANTIATE_TEST_SUITE_P(I8,
                         TestDivideByZeroCrashes,
                         ::testing::Values(fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {10_i8, 0_i8}},
                                           fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV},
                                                     .constants = {fc::Value{static_cast<int8_t>(-10)}, 0_i8}},
                                           fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {0_i8, 0_i8}},
                                           fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {127_i8, 0_i8}},
                                           fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV},
                                                     .constants = {fc::Value{static_cast<int8_t>(-128)}, 0_i8}}));
INSTANTIATE_TEST_SUITE_P(
  I16,
  TestDivideByZeroCrashes,
  ::testing::Values(
    fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {1000_i16, 0_i16}},
    fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {fc::Value{static_cast<int16_t>(-1000)}, 0_i16}},
    fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {0_i16, 0_i16}},
    fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {32767_i16, 0_i16}},
    fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {fc::Value{static_cast<int16_t>(-32768)}, 0_i16}}));
INSTANTIATE_TEST_SUITE_P(
  I32,
  TestDivideByZeroCrashes,
  ::testing::Values(fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {100000_i32, 0_i32}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV},
                              .constants = {fc::Value{static_cast<int32_t>(-100000)}, 0_i32}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {0_i32, 0_i32}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {2147483647_i32, 0_i32}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV},
                              .constants = {fc::Value{static_cast<int32_t>(-2147483648)}, 0_i32}}));
INSTANTIATE_TEST_SUITE_P(
  I64,
  TestDivideByZeroCrashes,
  ::testing::Values(fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {1000000000_i64, 0_i64}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV},
                              .constants = {fc::Value{static_cast<int64_t>(-1000000000)}, 0_i64}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {0_i64, 0_i64}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV}, .constants = {9223372036854775807_i64, 0_i64}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV},
                              .constants = {fc::Value{static_cast<int64_t>(-9223372036854775807LL - 1)}, 0_i64}}));
INSTANTIATE_TEST_SUITE_P(U8,
                         TestDivideByZeroCrashes,
                         ::testing::Values(fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {10_u8, 0_u8}},
                                           fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {0_u8, 0_u8}},
                                           fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {255_u8, 0_u8}},
                                           fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV},
                                                     .constants = {128_u8, 0_u8}}));
INSTANTIATE_TEST_SUITE_P(
  U16,
  TestDivideByZeroCrashes,
  ::testing::Values(fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {1000_u16, 0_u16}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {0_u16, 0_u16}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {65535_u16, 0_u16}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {32768_u16, 0_u16}}));
INSTANTIATE_TEST_SUITE_P(
  U32,
  TestDivideByZeroCrashes,
  ::testing::Values(fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {100000_u32, 0_u32}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {0_u32, 0_u32}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {4294967295_u32, 0_u32}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {2147483648_u32, 0_u32}}));
INSTANTIATE_TEST_SUITE_P(
  U64,
  TestDivideByZeroCrashes,
  ::testing::Values(fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {1000000000_u64, 0_u64}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {0_u64, 0_u64}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {18446744073709551615_u64, 0_u64}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV}, .constants = {9223372036854775808_u64, 0_u64}}));
