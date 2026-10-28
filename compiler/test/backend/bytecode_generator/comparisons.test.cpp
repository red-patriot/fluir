#include <gtest/gtest.h>

#include "ast_builders.hpp"
#include "bytecode/primitives.hpp"
#include "fixture.hpp"

namespace fc = fluir::code;
using namespace fluir::literals_types;
using namespace fluir::test;

TEST_F(TestBytecodeGenerator, GeneratesEqualityComparisons) {
  fc::Chunk expectedChunk{
    .name = "eq",
    .code =
      {
        fc::RESERVE,
        0x0,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::EQ,
        fc::Instruction::POP,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::EQ,
        fc::Instruction::NOT,
        fc::Instruction::POP,
        fc::Instruction::PUSH,
        0x02,
        fc::Instruction::PUSH,
        0x03,
        fc::Instruction::EQ,
        fc::Instruction::POP,
        fc::Instruction::PUSH,
        0x04,
        fc::Instruction::PUSH,
        0x05,
        fc::Instruction::EQ,
        fc::Instruction::POP,
        fc::Instruction::RETURN,
      },
  };

  auto writer = generate(
    program(function(3,
                     "eq",
                     binary(fluir::Operator::EQUAL_EQUAL, constant(I32{8}, {3, 1}), constant(I32{16}, {3, 2}), {3, 3}),
                     binary(fluir::Operator::BANG_EQUAL, constant(I32{8}, {3, 1}), constant(I32{16}, {3, 2}), {3, 3}),
                     binary(fluir::Operator::EQUAL_EQUAL, constant(1.5, {3, 1}), constant(2.5, {3, 2}), {3, 3}),
                     binary(fluir::Operator::EQUAL_EQUAL, constant(true, {3, 1}), constant(false, {3, 2}), {3, 3}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesSignedComparisons) {
  fc::Chunk expectedChunk{
    .name = "cmp",
    .code =
      {
        fc::RESERVE,
        0x0,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::I64_LT,
        fc::POP,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::I64_LE,
        fc::POP,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::I64_LT,
        fc::POP,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::I64_LE,
        fc::POP,
        fc::Instruction::RETURN,
      },
  };

  auto writer = generate(program(
    function(3,
             "cmp",
             binary(fluir::Operator::LESS, constant(I32{8}, {3, 1}), constant(I32{16}, {3, 2}), {3, 3}),
             binary(fluir::Operator::LESS_EQUAL, constant(I32{8}, {3, 1}), constant(I32{16}, {3, 2}), {3, 3}),
             binary(fluir::Operator::GREATER, constant(I32{8}, {3, 1}), constant(I32{16}, {3, 2}), {3, 3}),
             binary(fluir::Operator::GREATER_EQUAL, constant(I32{8}, {3, 1}), constant(I32{16}, {3, 2}), {3, 3}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesUnsignedComparisons) {
  fc::Chunk expectedChunk{
    .name = "cmp",
    .code =
      {
        fc::RESERVE,
        0x0,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::U64_LT,
        fc::POP,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::U64_LE,
        fc::POP,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::U64_LT,
        fc::POP,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::U64_LE,
        fc::POP,
        fc::Instruction::RETURN,
      },
  };

  auto writer = generate(program(
    function(3,
             "cmp",
             binary(fluir::Operator::LESS, constant(U32{8}, {3, 1}), constant(U32{16}, {3, 2}), {3, 3}),
             binary(fluir::Operator::LESS_EQUAL, constant(U32{8}, {3, 1}), constant(U32{16}, {3, 2}), {3, 3}),
             binary(fluir::Operator::GREATER, constant(U32{8}, {3, 1}), constant(U32{16}, {3, 2}), {3, 3}),
             binary(fluir::Operator::GREATER_EQUAL, constant(U32{8}, {3, 1}), constant(U32{16}, {3, 2}), {3, 3}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesFloatComparisons) {
  fc::Chunk expectedChunk{
    .name = "cmp",
    .code =
      {
        fc::RESERVE,
        0x0,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::F64_LT,
        fc::POP,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::F64_LE,
        fc::POP,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::F64_LT,
        fc::POP,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::F64_LE,
        fc::POP,
        fc::Instruction::RETURN,
      },
  };

  auto writer = generate(
    program(function(3,
                     "cmp",
                     binary(fluir::Operator::LESS, constant(1.5, {3, 1}), constant(2.5, {3, 2}), {3, 3}),
                     binary(fluir::Operator::LESS_EQUAL, constant(1.5, {3, 1}), constant(2.5, {3, 2}), {3, 3}),
                     // GREATER swaps operands: rhs (2.5 = constant index 1) is pushed first.
                     binary(fluir::Operator::GREATER, constant(1.5, {3, 1}), constant(2.5, {3, 2}), {3, 3}),
                     // GREATER_EQUAL swaps operands: rhs (2.5 = constant index 1) is pushed first.
                     binary(fluir::Operator::GREATER_EQUAL, constant(1.5, {3, 1}), constant(2.5, {3, 2}), {3, 3}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesLogicalOperators) {
  fc::Chunk expectedChunk{
    .name = "logic",
    .code =
      {
        fc::RESERVE,           0x0,     fc::Instruction::PUSH,   0x00,
        fc::Instruction::PUSH, 0x01,    fc::Instruction::AND,    fc::POP,
        fc::Instruction::PUSH, 0x00,    fc::Instruction::PUSH,   0x01,
        fc::Instruction::OR,   fc::POP, fc::Instruction::PUSH,   0x00,
        fc::Instruction::NOT,  fc::POP, fc::Instruction::RETURN,
      },
  };

  auto writer =
    generate(program(function(3,
                              "logic",
                              binary(fluir::Operator::AND_AND, constant(true, {3, 1}), constant(false, {3, 2}), {3, 3}),
                              binary(fluir::Operator::BAR_BAR, constant(true, {3, 1}), constant(false, {3, 2}), {3, 3}),
                              unary(fluir::Operator::BANG, constant(true, {3, 1}), {3, 2}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}
