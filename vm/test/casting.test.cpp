#include <array>
#include <climits>
#include <tuple>

#include <gtest/gtest.h>

#include "vm/vm.hpp"

using std::tuple;

namespace fc = fluir::code;
using enum fluir::code::Instruction;
using enum fluir::code::NumericWidth;
using namespace fluir::code::value_literals;

class TestCasting : public ::testing::TestWithParam<tuple<fc::Value, fc::Chunk>> { };

TEST_P(TestCasting, Test) {
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
  I8toU,
  TestCasting,
  ::testing::Values(
    // I8 to U8
    tuple{0_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {0_i8}}},
    tuple{127_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {127_i8}}},
    tuple{50_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {50_i8}}},
    tuple{128_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {fc::Value{static_cast<fc::I8>(-128)}}}},
    tuple{255_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {fc::Value{static_cast<fc::I8>(-1)}}}},
    // I8 to U16
    tuple{0_u16, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {0_i8}}},
    tuple{127_u16, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {127_i8}}},
    tuple{50_u16, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {50_i8}}},
    tuple{65408_u16,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {fc::Value{static_cast<fc::I8>(-128)}}}},
    tuple{65535_u16,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {fc::Value{static_cast<fc::I8>(-1)}}}},
    // I8 to U32
    tuple{0_u32, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {0_i8}}},
    tuple{127_u32, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {127_i8}}},
    tuple{50_u32, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {50_i8}}},
    tuple{4294967168_u32,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {fc::Value{static_cast<fc::I8>(-128)}}}},
    tuple{4294967295_u32,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {fc::Value{static_cast<fc::I8>(-1)}}}},
    // I8 to U64
    tuple{0_u64, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {0_i8}}},
    tuple{127_u64, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {127_i8}}},
    tuple{50_u64, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {50_i8}}},
    tuple{18446744073709551488_u64,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {fc::Value{static_cast<fc::I8>(-128)}}}},
    tuple{18446744073709551615_u64,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {fc::Value{static_cast<fc::I8>(-1)}}}}));

INSTANTIATE_TEST_SUITE_P(
  I16toU,
  TestCasting,
  ::testing::Values(
    // I16 to U8 (truncation)
    tuple{0_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {0_i16}}},
    tuple{255_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {32767_i16}}},
    tuple{50_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {1330_i16}}},
    tuple{0_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {fc::Value{static_cast<fc::I16>(-32768)}}}},
    tuple{255_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {fc::Value{static_cast<fc::I16>(-1)}}}},
    // I16 to U16
    tuple{0_u16, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {0_i16}}},
    tuple{32767_u16, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {32767_i16}}},
    tuple{1330_u16, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {1330_i16}}},
    tuple{32768_u16,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {fc::Value{static_cast<fc::I16>(-32768)}}}},
    tuple{65535_u16,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {fc::Value{static_cast<fc::I16>(-1)}}}},
    // I16 to U32
    tuple{0_u32, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {0_i16}}},
    tuple{32767_u32, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {32767_i16}}},
    tuple{1330_u32, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {1330_i16}}},
    tuple{4294934528_u32,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {fc::Value{static_cast<fc::I16>(-32768)}}}},
    tuple{4294967295_u32,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {fc::Value{static_cast<fc::I16>(-1)}}}},
    // I16 to U64
    tuple{0_u64, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {0_i16}}},
    tuple{32767_u64, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {32767_i16}}},
    tuple{1330_u64, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {1330_i16}}},
    tuple{18446744073709518848_u64,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {fc::Value{static_cast<fc::I16>(-32768)}}}},
    tuple{18446744073709551615_u64,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {fc::Value{static_cast<fc::I16>(-1)}}}}));

INSTANTIATE_TEST_SUITE_P(
  I32toU,
  TestCasting,
  ::testing::Values(
    // I32 to U8 (truncation)
    tuple{0_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {0_i32}}},
    tuple{255_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {2147483647_i32}}},
    tuple{160_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {100000_i32}}},
    tuple{0_u8,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {fc::Value{static_cast<fc::I32>(-2147483648)}}}},
    tuple{255_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {fc::Value{static_cast<fc::I32>(-1)}}}},
    // I32 to U16 (truncation)
    tuple{0_u16, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {0_i32}}},
    tuple{65535_u16, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {2147483647_i32}}},
    tuple{34464_u16, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {100000_i32}}},
    tuple{0_u16,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {fc::Value{static_cast<fc::I32>(-2147483648)}}}},
    tuple{65535_u16,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {fc::Value{static_cast<fc::I32>(-1)}}}},
    // I32 to U32
    tuple{0_u32, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {0_i32}}},
    tuple{2147483647_u32, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {2147483647_i32}}},
    tuple{100000_u32, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {100000_i32}}},
    tuple{2147483648_u32,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {fc::Value{static_cast<fc::I32>(-2147483648)}}}},
    tuple{4294967295_u32,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {fc::Value{static_cast<fc::I32>(-1)}}}},
    // I32 to U64
    tuple{0_u64, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {0_i32}}},
    tuple{2147483647_u64, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {2147483647_i32}}},
    tuple{100000_u64, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {100000_i32}}},
    tuple{18446744071562067968_u64,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {fc::Value{static_cast<fc::I32>(-2147483648)}}}},
    tuple{18446744073709551615_u64,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {fc::Value{static_cast<fc::I32>(-1)}}}}));

INSTANTIATE_TEST_SUITE_P(
  I64toU,
  TestCasting,
  ::testing::Values(
    // I64 to U8 (truncation)
    tuple{0_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {0_i64}}},
    tuple{255_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {9223372036854775807_i64}}},
    tuple{0_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {1000000000000_i64}}},
    tuple{0_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {fc::Value{INT64_MIN}}}},
    tuple{255_u8, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_8}, .constants = {fc::Value{static_cast<fc::I64>(-1)}}}},
    // I64 to U16 (truncation)
    tuple{0_u16, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {0_i64}}},
    tuple{65535_u16, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {9223372036854775807_i64}}},
    tuple{4096_u16, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {1000000000000_i64}}},
    tuple{0_u16, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {fc::Value{INT64_MIN}}}},
    tuple{65535_u16,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_16}, .constants = {fc::Value{static_cast<fc::I64>(-1)}}}},
    // I64 to U32 (truncation)
    tuple{0_u32, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {0_i64}}},
    tuple{4294967295_u32, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {9223372036854775807_i64}}},
    tuple{3567587328_u32, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {1000000000000_i64}}},
    tuple{0_u32, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {fc::Value{INT64_MIN}}}},
    tuple{4294967295_u32,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_32}, .constants = {fc::Value{static_cast<fc::I64>(-1)}}}},
    // I64 to U64
    tuple{0_u64, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {0_i64}}},
    tuple{9223372036854775807_u64,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {9223372036854775807_i64}}},
    tuple{1000000000000_u64, fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {1000000000000_i64}}},
    tuple{9223372036854775808_u64,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {fc::Value{INT64_MIN}}}},
    tuple{18446744073709551615_u64,
          fc::Chunk{.code = {PUSH, 0, CAST_IU, WIDTH_64}, .constants = {fc::Value{static_cast<fc::I64>(-1)}}}}));

INSTANTIATE_TEST_SUITE_P(
  U8toI,
  TestCasting,
  ::testing::Values(
    // U8 to I8
    tuple{0_i8, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {0_u8}}},
    tuple{127_i8, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {127_u8}}},
    tuple{50_i8, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {50_u8}}},
    tuple{fc::Value{static_cast<fc::I8>(-128)}, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {128_u8}}},
    tuple{fc::Value{static_cast<fc::I8>(-1)}, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {255_u8}}},
    // U8 to I16
    tuple{0_i16, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {0_u8}}},
    tuple{127_i16, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {127_u8}}},
    tuple{50_i16, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {50_u8}}},
    tuple{128_i16, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {128_u8}}},
    tuple{255_i16, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {255_u8}}},
    // U8 to I32
    tuple{0_i32, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {0_u8}}},
    tuple{127_i32, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {127_u8}}},
    tuple{50_i32, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {50_u8}}},
    tuple{128_i32, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {128_u8}}},
    tuple{255_i32, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {255_u8}}},
    // U8 to I64
    tuple{0_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {0_u8}}},
    tuple{127_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {127_u8}}},
    tuple{50_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {50_u8}}},
    tuple{128_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {128_u8}}},
    tuple{255_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {255_u8}}}));

INSTANTIATE_TEST_SUITE_P(
  U16toI,
  TestCasting,
  ::testing::Values(
    // U16 to I8 (truncation)
    tuple{0_i8, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {0_u16}}},
    tuple{fc::Value{static_cast<fc::I8>(-1)}, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {32767_u16}}},
    tuple{50_i8, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {1330_u16}}},
    tuple{0_i8, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {32768_u16}}},
    tuple{fc::Value{static_cast<fc::I8>(-1)}, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {65535_u16}}},
    // U16 to I16
    tuple{0_i16, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {0_u16}}},
    tuple{32767_i16, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {32767_u16}}},
    tuple{1330_i16, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {1330_u16}}},
    tuple{fc::Value{static_cast<fc::I16>(-32768)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {32768_u16}}},
    tuple{fc::Value{static_cast<fc::I16>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {65535_u16}}},
    // U16 to I32
    tuple{0_i32, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {0_u16}}},
    tuple{32767_i32, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {32767_u16}}},
    tuple{1330_i32, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {1330_u16}}},
    tuple{32768_i32, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {32768_u16}}},
    tuple{65535_i32, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {65535_u16}}},
    // U16 to I64
    tuple{0_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {0_u16}}},
    tuple{32767_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {32767_u16}}},
    tuple{1330_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {1330_u16}}},
    tuple{32768_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {32768_u16}}},
    tuple{65535_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {65535_u16}}}));

INSTANTIATE_TEST_SUITE_P(
  U32toI,
  TestCasting,
  ::testing::Values(
    // U32 to I8 (truncation)
    tuple{0_i8, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {0_u32}}},
    tuple{fc::Value{static_cast<fc::I8>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {2147483647_u32}}},
    tuple{fc::Value{static_cast<fc::I8>(-96)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {100000_u32}}},
    tuple{0_i8, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {2147483648_u32}}},
    tuple{fc::Value{static_cast<fc::I8>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {4294967295_u32}}},
    // U32 to I16 (truncation)
    tuple{0_i16, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {0_u32}}},
    tuple{fc::Value{static_cast<fc::I16>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {2147483647_u32}}},
    tuple{fc::Value{static_cast<fc::I16>(-31072)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {100000_u32}}},
    tuple{0_i16, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {2147483648_u32}}},
    tuple{fc::Value{static_cast<fc::I16>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {4294967295_u32}}},
    // U32 to I32
    tuple{0_i32, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {0_u32}}},
    tuple{2147483647_i32, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {2147483647_u32}}},
    tuple{100000_i32, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {100000_u32}}},
    tuple{fc::Value{static_cast<fc::I32>(-2147483648)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {2147483648_u32}}},
    tuple{fc::Value{static_cast<fc::I32>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {4294967295_u32}}},
    // U32 to I64
    tuple{0_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {0_u32}}},
    tuple{2147483647_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {2147483647_u32}}},
    tuple{100000_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {100000_u32}}},
    tuple{2147483648_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {2147483648_u32}}},
    tuple{4294967295_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {4294967295_u32}}}));

INSTANTIATE_TEST_SUITE_P(
  U64toI,
  TestCasting,
  ::testing::Values(
    // U64 to I8 (truncation)
    tuple{0_i8, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {0_u64}}},
    tuple{fc::Value{static_cast<fc::I8>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {9223372036854775807_u64}}},
    tuple{0_i8, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {1000000000000_u64}}},
    tuple{0_i8, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {9223372036854775808_u64}}},
    tuple{fc::Value{static_cast<fc::I8>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_8}, .constants = {18446744073709551615_u64}}},
    // U64 to I16 (truncation)
    tuple{0_i16, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {0_u64}}},
    tuple{fc::Value{static_cast<fc::I16>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {9223372036854775807_u64}}},
    tuple{4096_i16, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {1000000000000_u64}}},
    tuple{0_i16, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {9223372036854775808_u64}}},
    tuple{fc::Value{static_cast<fc::I16>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_16}, .constants = {18446744073709551615_u64}}},
    // U64 to I32 (truncation)
    tuple{0_i32, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {0_u64}}},
    tuple{fc::Value{static_cast<fc::I32>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {9223372036854775807_u64}}},
    tuple{fc::Value{static_cast<fc::I32>(-727379968)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {1000000000000_u64}}},
    tuple{0_i32, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {9223372036854775808_u64}}},
    tuple{fc::Value{static_cast<fc::I32>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_32}, .constants = {18446744073709551615_u64}}},
    // U64 to I64
    tuple{0_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {0_u64}}},
    tuple{9223372036854775807_i64,
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {9223372036854775807_u64}}},
    tuple{1000000000000_i64, fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {1000000000000_u64}}},
    tuple{fc::Value{INT64_MIN},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {9223372036854775808_u64}}},
    tuple{fc::Value{static_cast<fc::I64>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_UI, WIDTH_64}, .constants = {18446744073709551615_u64}}}));

INSTANTIATE_TEST_SUITE_P(
  I8toF,
  TestCasting,
  ::testing::Values(tuple{0.0, fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {0_i8}}},
                    tuple{127.0, fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {127_i8}}},
                    tuple{50.0, fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {50_i8}}},
                    tuple{fc::Value{static_cast<fc::F64>(-128.0)},
                          fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {fc::Value{static_cast<fc::I8>(-128)}}}},
                    tuple{fc::Value{static_cast<fc::F64>(-1.0)},
                          fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {fc::Value{static_cast<fc::I8>(-1)}}}}));

INSTANTIATE_TEST_SUITE_P(
  I16toF,
  TestCasting,
  ::testing::Values(tuple{0.0, fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {0_i16}}},
                    tuple{32767.0, fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {32767_i16}}},
                    tuple{1330.0, fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {1330_i16}}},
                    tuple{
                      fc::Value{static_cast<fc::F64>(-32768.0)},
                      fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {fc::Value{static_cast<fc::I16>(-32768)}}}},
                    tuple{fc::Value{static_cast<fc::F64>(-1.0)},
                          fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {fc::Value{static_cast<fc::I16>(-1)}}}}));

INSTANTIATE_TEST_SUITE_P(
  I32toF64,
  TestCasting,
  ::testing::Values(tuple{0.0, fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {0_i32}}},
                    tuple{2147483647.0, fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {2147483647_i32}}},
                    tuple{100000.0, fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {100000_i32}}},
                    tuple{fc::Value{static_cast<fc::F64>(-2147483648.0)},
                          fc::Chunk{.code = {PUSH, 0, CAST_IF},
                                    .constants = {fc::Value{static_cast<fc::I32>(-2147483648)}}}},
                    tuple{fc::Value{static_cast<fc::F64>(-1.0)},
                          fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {fc::Value{static_cast<fc::I32>(-1)}}}}));

INSTANTIATE_TEST_SUITE_P(
  I64toF64,
  TestCasting,
  ::testing::Values(tuple{0.0, fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {0_i64}}},
                    tuple{9223372036854775807.0,
                          fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {9223372036854775807_i64}}},
                    tuple{1000000000000.0, fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {1000000000000_i64}}},
                    tuple{fc::Value{static_cast<fc::F64>(-9223372036854775808.0)},
                          fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {fc::Value{INT64_MIN}}}},
                    tuple{fc::Value{static_cast<fc::F64>(-1.0)},
                          fc::Chunk{.code = {PUSH, 0, CAST_IF}, .constants = {fc::Value{static_cast<fc::I64>(-1)}}}}));

INSTANTIATE_TEST_SUITE_P(U8toF64,
                         TestCasting,
                         ::testing::Values(tuple{0.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {0_u8}}},
                                           tuple{127.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {127_u8}}},
                                           tuple{50.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {50_u8}}},
                                           tuple{128.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {128_u8}}},
                                           tuple{255.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {255_u8}}}));

INSTANTIATE_TEST_SUITE_P(
  U16toF64,
  TestCasting,
  ::testing::Values(tuple{0.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {0_u16}}},
                    tuple{32767.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {32767_u16}}},
                    tuple{1330.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {1330_u16}}},
                    tuple{32768.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {32768_u16}}},
                    tuple{65535.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {65535_u16}}}));

INSTANTIATE_TEST_SUITE_P(
  U32toF64,
  TestCasting,
  ::testing::Values(tuple{0.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {0_u32}}},
                    tuple{2147483647.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {2147483647_u32}}},
                    tuple{100000.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {100000_u32}}},
                    tuple{2147483648.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {2147483648_u32}}},
                    tuple{4294967295.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {4294967295_u32}}}));

INSTANTIATE_TEST_SUITE_P(
  U64toF64,
  TestCasting,
  ::testing::Values(
    tuple{0.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {0_u64}}},
    tuple{9223372036854775807.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {9223372036854775807_u64}}},
    tuple{1000000000000.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {1000000000000_u64}}},
    tuple{9223372036854775808.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {9223372036854775808_u64}}},
    tuple{18446744073709551615.0, fc::Chunk{.code = {PUSH, 0, CAST_UF}, .constants = {18446744073709551615_u64}}}));

INSTANTIATE_TEST_SUITE_P(
  F64toI8,
  TestCasting,
  ::testing::Values(
    tuple{0_i8, fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_8}, .constants = {0.0_f64}}},
    tuple{127_i8, fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_8}, .constants = {127.9_f64}}},
    tuple{50_i8, fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_8}, .constants = {50.5_f64}}},
    tuple{fc::Value{static_cast<fc::I8>(-128)},
          fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_8}, .constants = {fc::Value{static_cast<fc::F64>(-128.7)}}}},
    tuple{fc::Value{static_cast<fc::I8>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_8}, .constants = {fc::Value{static_cast<fc::F64>(-1.3)}}}}));

INSTANTIATE_TEST_SUITE_P(
  F64toI16,
  TestCasting,
  ::testing::Values(
    tuple{0_i16, fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_16}, .constants = {0.0_f64}}},
    tuple{32767_i16, fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_16}, .constants = {32767.8_f64}}},
    tuple{1330_i16, fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_16}, .constants = {1330.4_f64}}},
    tuple{fc::Value{static_cast<fc::I16>(-32768)},
          fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_16}, .constants = {fc::Value{static_cast<fc::F64>(-32768.2)}}}},
    tuple{fc::Value{static_cast<fc::I16>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_16}, .constants = {fc::Value{static_cast<fc::F64>(-1.9)}}}}));

INSTANTIATE_TEST_SUITE_P(
  F64toI32,
  TestCasting,
  ::testing::Values(
    tuple{0_i32, fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_32}, .constants = {0.0_f64}}},
    tuple{2147483647_i32, fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_32}, .constants = {2147483647.0_f64}}},
    tuple{100000_i32, fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_32}, .constants = {100000.6_f64}}},
    tuple{
      fc::Value{static_cast<fc::I32>(-2147483648)},
      fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_32}, .constants = {fc::Value{static_cast<fc::F64>(-2147483648.0)}}}},
    tuple{fc::Value{static_cast<fc::I32>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_32}, .constants = {fc::Value{static_cast<fc::F64>(-1.1)}}}}));

INSTANTIATE_TEST_SUITE_P(
  F64toI64,
  TestCasting,
  ::testing::Values(
    tuple{0_i64, fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_64}, .constants = {0.0_f64}}},
    tuple{fc::Value{INT64_MIN},  // This case wraps around to INT64_MIN because of FP epsilon funsies
          fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_64}, .constants = {9223372036854775807.0_f64}}},
    tuple{1000000000000_i64, fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_64}, .constants = {1000000000000.7_f64}}},
    tuple{fc::Value{INT64_MIN},
          fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_64},
                    .constants = {fc::Value{static_cast<fc::F64>(-9223372036854775808.0)}}}},
    tuple{fc::Value{static_cast<fc::I64>(-1)},
          fc::Chunk{.code = {PUSH, 0, CAST_FI, WIDTH_64}, .constants = {fc::Value{static_cast<fc::F64>(-1.5)}}}}));

// ============================================================================
// F64 to U* (float to unsigned)
// ============================================================================
INSTANTIATE_TEST_SUITE_P(
  F64toU8,
  TestCasting,
  ::testing::Values(tuple{0_u8, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_8}, .constants = {0.0_f64}}},
                    tuple{127_u8, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_8}, .constants = {127.9_f64}}},
                    tuple{50_u8, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_8}, .constants = {50.5_f64}}},
                    tuple{128_u8, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_8}, .constants = {128.3_f64}}},
                    tuple{255_u8, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_8}, .constants = {255.8_f64}}}));

INSTANTIATE_TEST_SUITE_P(
  F64toU16,
  TestCasting,
  ::testing::Values(tuple{0_u16, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_16}, .constants = {0.0_f64}}},
                    tuple{32767_u16, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_16}, .constants = {32767.6_f64}}},
                    tuple{1330_u16, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_16}, .constants = {1330.2_f64}}},
                    tuple{32768_u16, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_16}, .constants = {32768.4_f64}}},
                    tuple{65535_u16, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_16}, .constants = {65535.9_f64}}}));

INSTANTIATE_TEST_SUITE_P(
  F64toU32,
  TestCasting,
  ::testing::Values(
    tuple{0_u32, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_32}, .constants = {0.0_f64}}},
    tuple{2147483647_u32, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_32}, .constants = {2147483647.0_f64}}},
    tuple{100000_u32, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_32}, .constants = {100000.1_f64}}},
    tuple{2147483648_u32, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_32}, .constants = {2147483648.0_f64}}},
    tuple{4294967295_u32, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_32}, .constants = {4294967295.0_f64}}}));

INSTANTIATE_TEST_SUITE_P(
  F64toU64,
  TestCasting,
  ::testing::Values(tuple{0_u64, fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_64}, .constants = {0.0_f64}}},
                    tuple{9223372036854775808_u64,
                          fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_64}, .constants = {9223372036854775807.0_f64}}},
                    tuple{1000000000000_u64,
                          fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_64}, .constants = {1000000000000.3_f64}}},
                    tuple{9223372036854775808_u64,
                          fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_64}, .constants = {9223372036854775808.0_f64}}},
                    tuple{0_u64,  /// This case wraps around to 0 because of FP epsilon funsies
                          fc::Chunk{.code = {PUSH, 0, CAST_FU, WIDTH_64}, .constants = {18446744073709551615.0_f64}}}));
