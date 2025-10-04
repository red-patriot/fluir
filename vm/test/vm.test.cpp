#include "vm/vm.hpp"

#include <array>

#include <gtest/gtest.h>

namespace fc = fluir::code;
using enum fluir::code::Instruction;
using namespace fluir::code::value_literals;

TEST(TestVM, ExecEmptyFunction) {
  fluir::code::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {EXIT}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
}

TEST(TestVM, ExecuteSimpleAddition) {
  fluir::code::ByteCode code{
    .header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_ADD, EXIT}, .constants = {1.2_f64, 2.5_f64}}}};
  double expected = 3.7;

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_DOUBLE_EQ(expected, uut.viewStack().back().asF64());
}

TEST(TestVM, ExecuteSimpleSubtraction) {
  fluir::code::ByteCode code{
    .header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_SUB, EXIT}, .constants = {1.2_f64, 2.5_f64}}}};
  double expected = -1.3;

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_DOUBLE_EQ(expected, uut.viewStack().back().asF64());
}

TEST(TestVM, ExecuteSimpleMultiply) {
  fluir::code::ByteCode code{
    .header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_MUL, EXIT}, .constants = {1.2_f64, 2.5_f64}}}};
  double expected = 3.0;

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_DOUBLE_EQ(expected, uut.viewStack().back().asF64());
}

TEST(TestVM, ExecuteSimpleDivide) {
  fluir::code::ByteCode code{
    .header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_DIV, EXIT}, .constants = {1.2_f64, 2.5_f64}}}};
  double expected = 0.48;

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_DOUBLE_EQ(expected, uut.viewStack().back().asF64());
}

TEST(TestVM, PushAndPopCorrectly) {
  fluir::code::ByteCode code{
    .header = {},
    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_DIV, POP, EXIT}, .constants = {1.2_f64, 2.5_f64}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(0, uut.viewStack().size());
}

TEST(TestVM, AddI64) {
  std::int64_t expected = 37;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD, EXIT}, .constants = {12_i64, 25_i64}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI64());
}

TEST(TestVM, SubtractI64) {
  std::int64_t expected = -13;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB, EXIT}, .constants = {12_i64, 25_i64}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI64());
}

TEST(TestVM, MultiplyI64) {
  std::int64_t expected = 10;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL, EXIT}, .constants = {2_i64, 5_i64}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI64());
}

TEST(TestVM, DivideI64) {
  std::int64_t expected = 3;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV, EXIT}, .constants = {12_i64, 4_i64}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI64());
}

TEST(TestVM, NegateI64) {
  std::int64_t expected = -5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, I64_NEG, EXIT}, .constants = {5_i64}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI64());
}

TEST(TestVM, AffirmI64) {
  std::int64_t expected = 5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, I64_AFF, EXIT}, .constants = {5_i64}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI64());
}

TEST(TestVM, AddI32) {
  std::int32_t expected = 37;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD, EXIT}, .constants = {12_i32, 25_i32}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI32());
}

TEST(TestVM, SubtractI32) {
  std::int32_t expected = -13;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB, EXIT}, .constants = {12_i32, 25_i32}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI32());
}

TEST(TestVM, MultiplyI32) {
  std::int32_t expected = 10;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL, EXIT}, .constants = {2_i32, 5_i32}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI32());
}

TEST(TestVM, DivideI32) {
  std::int32_t expected = 3;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV, EXIT}, .constants = {12_i32, 4_i32}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI32());
}

TEST(TestVM, NegateI32) {
  std::int32_t expected = -5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, I64_NEG, EXIT}, .constants = {5_i32}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI32());
}

TEST(TestVM, AffirmI32) {
  std::int32_t expected = 5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, I64_AFF, EXIT}, .constants = {5_i32}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI32());
}

TEST(TestVM, AddI16) {
  std::int16_t expected = 37;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD, EXIT}, .constants = {12_i16, 25_i16}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI16());
}

TEST(TestVM, SubtractI16) {
  std::int16_t expected = -13;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB, EXIT}, .constants = {12_i16, 25_i16}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI16());
}

TEST(TestVM, MultiplyI16) {
  std::int16_t expected = 10;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL, EXIT}, .constants = {2_i16, 5_i16}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI16());
}

TEST(TestVM, DivideI16) {
  std::int16_t expected = 3;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV, EXIT}, .constants = {12_i16, 4_i16}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI16());
}

TEST(TestVM, NegateI16) {
  std::int16_t expected = -5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, I64_NEG, EXIT}, .constants = {5_i16}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI16());
}

TEST(TestVM, AffirmI16) {
  std::int16_t expected = 5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, I64_AFF, EXIT}, .constants = {5_i16}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI16());
}

TEST(TestVM, AddI8) {
  std::int8_t expected = 37;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_ADD, EXIT}, .constants = {12_i8, 25_i8}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI8());
}

TEST(TestVM, SubtractI8) {
  std::int8_t expected = -13;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_SUB, EXIT}, .constants = {12_i8, 25_i8}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI8());
}

TEST(TestVM, MultiplyI8) {
  std::int8_t expected = 10;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_MUL, EXIT}, .constants = {2_i8, 5_i8}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI8());
}

TEST(TestVM, DivideI8) {
  std::int8_t expected = 3;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, I64_DIV, EXIT}, .constants = {12_i8, 4_i8}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI8());
}

TEST(TestVM, NegateI8) {
  std::int8_t expected = -5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, I64_NEG, EXIT}, .constants = {5_i8}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI8());
}

TEST(TestVM, AffirmI8) {
  std::int8_t expected = 5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, I64_AFF, EXIT}, .constants = {5_i8}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asI8());
}

TEST(TestVM, AddU64) {
  std::uint64_t expected = 37;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD, EXIT}, .constants = {12_u64, 25_u64}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU64());
}

TEST(TestVM, SubtractU64) {
  std::uint64_t expected = -13;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB, EXIT}, .constants = {12_u64, 25_u64}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU64());
}

TEST(TestVM, MultiplyU64) {
  std::uint64_t expected = 10;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL, EXIT}, .constants = {2_u64, 5_u64}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU64());
}

TEST(TestVM, DivideU64) {
  std::uint64_t expected = 3;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV, EXIT}, .constants = {12_u64, 4_u64}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU64());
}

TEST(TestVM, NegateU64) {
  std::uint64_t expected = -5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, U64_NEG, EXIT}, .constants = {5_u64}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU64());
}

TEST(TestVM, AffirmU64) {
  std::uint64_t expected = 5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, U64_AFF, EXIT}, .constants = {5_u64}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU64());
}

TEST(TestVM, AddU32) {
  std::uint32_t expected = 37;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD, EXIT}, .constants = {12_u32, 25_u32}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU32());
}

TEST(TestVM, SubtractU32) {
  std::uint32_t expected = -13;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB, EXIT}, .constants = {12_u32, 25_u32}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU32());
}

TEST(TestVM, MultiplyU32) {
  std::uint32_t expected = 10;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL, EXIT}, .constants = {2_u32, 5_u32}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU32());
}

TEST(TestVM, DivideU32) {
  std::uint32_t expected = 3;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV, EXIT}, .constants = {12_u32, 4_u32}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU32());
}

TEST(TestVM, NegateU32) {
  std::uint32_t expected = -5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, U64_NEG, EXIT}, .constants = {5_u32}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU32());
}

TEST(TestVM, AffirmU32) {
  std::uint32_t expected = 5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, U64_AFF, EXIT}, .constants = {5_u32}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU32());
}

TEST(TestVM, AddU16) {
  std::uint16_t expected = 37;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD, EXIT}, .constants = {12_u16, 25_u16}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU16());
}

TEST(TestVM, SubtractU16) {
  std::uint16_t expected = -13;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB, EXIT}, .constants = {12_u16, 25_u16}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU16());
}

TEST(TestVM, MultiplyU16) {
  std::uint16_t expected = 10;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL, EXIT}, .constants = {2_u16, 5_u16}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU16());
}

TEST(TestVM, DivideU16) {
  std::uint16_t expected = 3;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV, EXIT}, .constants = {12_u16, 4_u16}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU16());
}

TEST(TestVM, NegateU16) {
  std::uint16_t expected = -5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, U64_NEG, EXIT}, .constants = {5_u16}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU16());
}

TEST(TestVM, AffirmU16) {
  std::uint16_t expected = 5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, U64_AFF, EXIT}, .constants = {5_u16}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU16());
}

TEST(TestVM, AddU8) {
  std::uint8_t expected = 37;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_ADD, EXIT}, .constants = {12_u8, 25_u8}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU8());
}

TEST(TestVM, SubtractU8) {
  std::uint8_t expected = -13;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_SUB, EXIT}, .constants = {12_u8, 25_u8}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU8());
}

TEST(TestVM, MultiplyU8) {
  std::uint8_t expected = 10;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_MUL, EXIT}, .constants = {2_u8, 5_u8}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU8());
}

TEST(TestVM, DivideU8) {
  std::uint8_t expected = 3;

  fc::ByteCode code{.header = {},
                    .chunks = {fc::Chunk{.code = {PUSH, 0, PUSH, 1, U64_DIV, EXIT}, .constants = {12_u8, 4_u8}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU8());
}

TEST(TestVM, NegateU8) {
  std::uint8_t expected = -5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, U64_NEG, EXIT}, .constants = {5_u8}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU8());
}

TEST(TestVM, AffirmU8) {
  std::uint8_t expected = 5;
  fc::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {PUSH, 0, U64_AFF, EXIT}, .constants = {5_u8}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().back().asU8());
}

// TODO: Tests for error cases
