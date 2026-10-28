#include <gtest/gtest.h>

#include "ast_builders.hpp"
#include "bytecode/primitives.hpp"
#include "fixture.hpp"

namespace fc = fluir::code;
using namespace fluir::literals_types;
using namespace fluir::test;

TEST_F(TestBytecodeGenerator, GeneratesSimpleBinaryExpression) {
  fc::Chunk expectedChunk{
    .name = "foo",
    .code =
      {
        fc::RESERVE,
        0x0,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::F64_MUL,
        fc::Instruction::POP,
        fc::Instruction::RETURN,
      },
  };

  auto writer = generate(
    program(function(3, "foo", binary(fluir::Operator::STAR, constant(1.5, {3, 3}), constant(2.5, {3, 2}), {3, 1}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesSimpleUnaryExpression) {
  fc::Chunk expectedChunk{.name = "bar",
                          .code = {
                            fc::RESERVE,
                            0x0,
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::F64_NEG,
                            fc::Instruction::POP,
                            fc::Instruction::RETURN,
                          }};

  auto writer = generate(program(function(3, "bar", unary(fluir::Operator::MINUS, constant(3.456, {3, 3}), {3, 1}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesIntBinaryExpression) {
  fc::Chunk expectedChunk{.name = "ints",
                          .code = {
                            fc::RESERVE,
                            0x0,
                            fc::Instruction::PUSH,
                            0x0,
                            fc::Instruction::PUSH,
                            0x01,
                            fc::Instruction::I64_ADD,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x2,
                            fc::Instruction::PUSH,
                            0x01,
                            fc::Instruction::I64_SUB,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x0,
                            fc::Instruction::PUSH,
                            0x01,
                            fc::Instruction::I64_MUL,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x0,
                            fc::Instruction::PUSH,
                            0x01,
                            fc::Instruction::I64_DIV,
                            fc::Instruction::POP,
                            fc::Instruction::RETURN,
                          }};

  auto writer = generate(
    program(function(3,
                     "ints",
                     binary(fluir::Operator::PLUS, constant(I32{8}, {3, 1}), constant(I32{16}, {3, 2}), {3, 3}),
                     binary(fluir::Operator::MINUS, constant(I32{28}, {3, 1}), constant(I32{16}, {3, 2}), {3, 3}),
                     binary(fluir::Operator::STAR, constant(I32{8}, {3, 1}), constant(I32{16}, {3, 2}), {3, 3}),
                     binary(fluir::Operator::SLASH, constant(I32{8}, {3, 1}), constant(I32{16}, {3, 2}), {3, 3}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesUintBinaryExpression) {
  fc::Chunk expectedChunk{
    .name = "ints",
    .code =
      {
        fc::RESERVE,
        0x0,
        fc::Instruction::PUSH,
        0x0,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::U64_ADD,
        fc::Instruction::POP,
        fc::Instruction::PUSH,
        0x2,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::U64_SUB,
        fc::Instruction::POP,
        fc::Instruction::PUSH,
        0x0,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::U64_MUL,
        fc::Instruction::POP,
        fc::Instruction::PUSH,
        0x0,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::U64_DIV,
        fc::Instruction::POP,
        fc::Instruction::RETURN,
      },
  };

  auto writer = generate(
    program(function(3,
                     "ints",
                     binary(fluir::Operator::PLUS, constant(U32{8}, {3, 1}), constant(U32{16}, {3, 2}), {3, 3}),
                     binary(fluir::Operator::MINUS, constant(U32{28}, {3, 1}), constant(U32{16}, {3, 2}), {3, 3}),
                     binary(fluir::Operator::STAR, constant(U32{8}, {3, 1}), constant(U32{16}, {3, 2}), {3, 3}),
                     binary(fluir::Operator::SLASH, constant(U32{8}, {3, 1}), constant(U32{16}, {3, 2}), {3, 3}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesIncrementDecrementOperations) {
  fc::Chunk expectedChunk{
    .name = "inc_dec",
    .code =
      {
        fc::RESERVE, 0x0,      fc::PUSH,    0x0,         fc::I64_INC, fc::POP,  fc::PUSH,    0x0,         fc::I64_DEC,
        fc::POP,     fc::PUSH, 0x1,         fc::F64_INC, fc::POP,     fc::PUSH, 0x1,         fc::F64_DEC, fc::POP,
        fc::PUSH,    0x2,      fc::U64_INC, fc::POP,     fc::PUSH,    0x2,      fc::U64_DEC, fc::POP,     fc::RETURN,
      },
  };

  auto writer = generate(program(function(3,
                                          "inc_dec",
                                          unary(fluir::Operator::PLUS_PLUS, constant(I32{8}, {3, 1}), {3, 4}),
                                          unary(fluir::Operator::MINUS_MINUS, constant(I32{8}, {3, 1}), {3, 5}),
                                          unary(fluir::Operator::PLUS_PLUS, constant(12.45, {3, 2}), {3, 6}),
                                          unary(fluir::Operator::MINUS_MINUS, constant(12.45, {3, 2}), {3, 7}),
                                          unary(fluir::Operator::PLUS_PLUS, constant(U64{8}, {3, 3}), {3, 8}),
                                          unary(fluir::Operator::MINUS_MINUS, constant(U64{8}, {3, 3}), {3, 9}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}
