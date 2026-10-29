#include <gtest/gtest.h>

#include "ast_builders.hpp"
#include "bytecode/primitives.hpp"
#include "compiler/utility/diagnostic/internal_error.hpp"
#include "fixture.hpp"

namespace fc = fluir::code;
using namespace fluir::test;

using fluir::FullID;

TEST_F(TestBytecodeGenerator, GeneratesExpressionWithSharedNodes) {
  const FullID SHARED_ID{3, 4};
  auto shared = localWrite(binary(fluir::Operator::SLASH,
                                  unary(fluir::Operator::MINUS, constant(3.5, {3, 5}), {3, 2}),
                                  constant(4.4, {3, 6}),
                                  SHARED_ID));
  auto binaryDependent =
    binary(fluir::Operator::PLUS, constant(100.0, {3, 3}), localRead(SHARED_ID.back(), {3, 1}), {3, 1});
  auto unaryDependent = unary(fluir::Operator::MINUS, localRead(SHARED_ID.back(), {3, 7}), {3, 7});

  fc::Chunk expectedChunk{.name = "bar",
                          .code = {
                            fc::RESERVE, 0x1,     fc::PUSH,     0x0, fc::F64_NEG, fc::PUSH, 0x1,
                            fc::F64_DIV,  // write {3,4}
                            fc::SET_VAL, 0x0,     fc::PUSH,     0x2, fc::GET_VAL,
                            0x0,  // Read {3,4}
                            fc::F64_ADD, fc::POP, fc::GET_VAL,
                            0x0,  // Read {3,4}
                            fc::F64_NEG, fc::POP, fc::MULTIPOP, 0x1, fc::RETURN,
                          }};

  auto writer =
    generate(program(function(3, "bar", std::move(shared), std::move(binaryDependent), std::move(unaryDependent))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesExpressionWithMultipleSharedNodes) {
  const FullID SHARED_1{1, 1};
  const FullID SHARED_2{1, 6};
  auto shared1 = localWrite(unary(
    fluir::Operator::MINUS, binary(fluir::Operator::STAR, constant(2, {1, 4}), constant(3, {1, 5}), {1, 3}), SHARED_1));
  auto shared2 =
    localWrite(binary(fluir::Operator::STAR, localRead(SHARED_1.back(), SHARED_2), constant(2, {1, 8}), SHARED_2));
  auto sum =
    binary(fluir::Operator::PLUS, localRead(SHARED_2.back(), {1, 8}), localRead(SHARED_1.back(), {1, 8}), {1, 8});

  fc::Chunk expectedChunk{
    .name = "main",
    .code = {
      fc::RESERVE, 0x2,         fc::PUSH, 0x0,         fc::PUSH, 0x1,          fc::I64_MUL, fc::I64_NEG, fc::SET_VAL,
      0x0,         fc::GET_VAL, 0x0,      fc::PUSH,    0x0,      fc::I64_MUL,  fc::SET_VAL, 0x1,         fc::GET_VAL,
      0x1,         fc::GET_VAL, 0x0,      fc::I64_ADD, fc::POP,  fc::MULTIPOP, 0x2,         fc::RETURN,
    }};

  auto writer = generate(program(function(1, "main", std::move(shared1), std::move(shared2), std::move(sum))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, HandlesMissingLocalVariable) {
  auto input = program(function(3, "inc_dec", localRead(2, {3, 1})));

  TestWriter writer;
  EXPECT_THROW(fluir::generateCode(ctx_, input, writer), fluir::diagnostic::InternalError);
}
