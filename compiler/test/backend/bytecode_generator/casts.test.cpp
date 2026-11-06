#include <gtest/gtest.h>

#include "ast_builders.hpp"
#include "bytecode/primitives.hpp"
#include "fixture.hpp"

namespace fc = fluir::code;
namespace ft = fluir::types;
using namespace fluir::literals_types;
using namespace fluir::test;

TEST_F(TestBytecodeGenerator, GeneratesIntCasts) {
  fc::Chunk expectedChunk{.name = "ints",
                          .code = {
                            fc::RESERVE,
                            0x0,
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::CAST_IF,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x01,
                            fc::Instruction::CAST_FI,
                            fc::NumericWidth::WIDTH_64,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::CAST_IU,
                            fc::NumericWidth::WIDTH_64,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::CAST_WIDTH,
                            fc::NumericWidth::WIDTH_8,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::CAST_WIDTH,
                            fc::NumericWidth::WIDTH_16,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::CAST_WIDTH,
                            fc::NumericWidth::WIDTH_64,
                            fc::Instruction::POP,
                            fc::Instruction::RETURN,
                          }};

  auto writer = generate(program(function(3,
                                          "ints",
                                          cast(ft::ID_F64, constant(I32{8}, {3, 1}), {3, 2}),
                                          cast(ft::ID_I64, constant(12.4, {3, 3}), {3, 3}),
                                          cast(ft::ID_U64, constant(I32{8}, {3, 1}), {3, 2}),
                                          cast(ft::ID_I8, constant(I32{8}, {3, 1}), {3, 2}),
                                          cast(ft::ID_I16, constant(I32{8}, {3, 1}), {3, 2}),
                                          cast(ft::ID_I64, constant(I32{8}, {3, 1}), {3, 2}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesIntToUintCastsWithWidthCasts) {
  fc::Chunk expectedChunk{.name = "ints",
                          .code = {
                            fc::RESERVE,
                            0x0,
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::CAST_IU,
                            fc::NumericWidth::WIDTH_32,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::CAST_IU,
                            fc::NumericWidth::WIDTH_16,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::CAST_IU,
                            fc::NumericWidth::WIDTH_8,
                            fc::Instruction::POP,
                            fc::Instruction::RETURN,
                          }};

  auto writer = generate(program(function(3,
                                          "ints",
                                          cast(ft::ID_U32, constant(I32{8}, {3, 1}), {3, 2}),
                                          cast(ft::ID_U16, constant(I32{8}, {3, 1}), {3, 2}),
                                          cast(ft::ID_U8, constant(I32{8}, {3, 1}), {3, 2}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesUintCasts) {
  fc::Chunk expectedChunk{.name = "uints",
                          .code = {
                            fc::RESERVE,
                            0x0,
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::CAST_UF,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x01,
                            fc::Instruction::CAST_FU,
                            fc::NumericWidth::WIDTH_64,
                            fc::Instruction::POP,
                            fc::Instruction::RETURN,
                          }};

  auto writer = generate(program(function(
    3, "uints", cast(ft::ID_F64, constant(U32{8}, {3, 1}), {3, 2}), cast(ft::ID_U64, constant(12.4, {3, 3}), {3, 3}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesUintToIntCastsWithWidthCasts) {
  fc::Chunk expectedChunk{.name = "uints",
                          .code = {
                            fc::RESERVE,
                            0x0,
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::CAST_UI,
                            fc::NumericWidth::WIDTH_32,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::CAST_UI,
                            fc::NumericWidth::WIDTH_16,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::CAST_UI,
                            fc::NumericWidth::WIDTH_8,
                            fc::Instruction::POP,
                            fc::Instruction::RETURN,
                          }};

  auto writer = generate(program(function(3,
                                          "uints",
                                          cast(ft::ID_I32, constant(U32{8}, {3, 1}), {3, 2}),
                                          cast(ft::ID_I16, constant(U32{8}, {3, 1}), {3, 2}),
                                          cast(ft::ID_I8, constant(U32{8}, {3, 1}), {3, 2}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}
