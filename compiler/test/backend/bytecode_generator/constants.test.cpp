#include <gtest/gtest.h>

#include "ast_builders.hpp"
#include "bytecode/primitives.hpp"
#include "fixture.hpp"

namespace fc = fluir::code;
using namespace fluir::literals_types;
using namespace fluir::test;

TEST_F(TestBytecodeGenerator, GeneratesIntConstants) {
  fc::Chunk expectedChunk{.name = "ints",
                          .code = {
                            fc::RESERVE,
                            0x0,
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x01,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x02,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x03,
                            fc::Instruction::POP,
                            fc::Instruction::RETURN,
                          }};

  auto writer = generate(program(function(3,
                                          "ints",
                                          constant(I8{8}, {3, 1}),
                                          constant(I16{16}, {3, 2}),
                                          constant(I32{32}, {3, 3}),
                                          constant(I64{64}, {3, 4}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesUintConstants) {
  fc::Chunk expectedChunk{.name = "ints",
                          .code = {
                            fc::RESERVE,
                            0x0,
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x01,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x02,
                            fc::Instruction::POP,
                            fc::Instruction::PUSH,
                            0x03,
                            fc::Instruction::POP,
                            fc::Instruction::RETURN,
                          }};

  auto writer = generate(program(function(3,
                                          "ints",
                                          constant(U8{8}, {3, 1}),
                                          constant(U16{16}, {3, 2}),
                                          constant(U32{32}, {3, 3}),
                                          constant(U64{64}, {3, 4}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, ConstantsAreGlobal) {
  // Global pool: [1.0, 2.0, 3.0, 4.0]
  // foo: PUSH 0, PUSH 1, F64_ADD, POP, RETURN
  // bar: PUSH 2, PUSH 3, F64_ADD, POP, RETURN
  fluir::be::ConstantsArray expectedConstants{1.0, 2.0, 3.0, 4.0};
  std::vector expectedChunks{
    fc::Chunk{.name = "foo",
              .code = {fc::RESERVE, 0x0, fc::PUSH, 0x00, fc::PUSH, 0x01, fc::F64_ADD, fc::POP, fc::RETURN}},
    fc::Chunk{.name = "bar",
              .code = {fc::RESERVE, 0x0, fc::PUSH, 0x02, fc::PUSH, 0x03, fc::F64_ADD, fc::POP, fc::RETURN}},
  };

  auto writer = generate(
    program(function(3, "foo", binary(fluir::Operator::PLUS, constant(1.0, {3, 2}), constant(2.0, {3, 3}), {3, 1})),
            function(4, "bar", binary(fluir::Operator::PLUS, constant(3.0, {4, 2}), constant(4.0, {4, 3}), {4, 1}))));

  EXPECT_BC_VALUES_EQ(expectedConstants, writer.constants);
  for (int i = 0; i != 2; ++i) {
    EXPECT_CHUNK_EQ(expectedChunks.at(i), writer.chunks.at(i));
  }
}

TEST_F(TestBytecodeGenerator, EmitsQPushForLotsOfConstants) {
  size_t constsCount = 257;
  auto foo = function(3, "foo");
  foo.statements.reserve(constsCount);
  for (size_t i = 0; i != constsCount; ++i) {
    foo.statements.emplace_back(constant(i, {i}));
  }

  fluir::be::ConstantsArray expectedConstants = [&]() {
    fluir::be::ConstantsArray values;
    values.reserve(constsCount);
    for (size_t i = 0; i != constsCount; ++i) {
      values.emplace_back(i);
    }
    return values;
  }();
  fc::Chunk expectedChunk{fc::Chunk{.name = "foo", .code = {[]() {
                                                     std::vector<uint8_t> bytes;
                                                     bytes.emplace_back(fc::RESERVE);
                                                     bytes.emplace_back(0x0);
                                                     for (size_t i = 0; i <= UINT8_MAX; ++i) {
                                                       bytes.emplace_back(fc::PUSH);
                                                       bytes.emplace_back(static_cast<uint8_t>(i));
                                                       bytes.emplace_back(fc::POP);
                                                     }
                                                     bytes.emplace_back(fc::QUAD_PUSH);
                                                     bytes.emplace_back(static_cast<uint8_t>(0));
                                                     bytes.emplace_back(static_cast<uint8_t>(0));
                                                     bytes.emplace_back(static_cast<uint8_t>(0x01));
                                                     bytes.emplace_back(static_cast<uint8_t>(0));
                                                     bytes.emplace_back(fc::POP);
                                                     bytes.emplace_back(fc::RETURN);
                                                     return bytes;
                                                   }()}}};

  auto writer = generate(program(std::move(foo)));

  EXPECT_BC_VALUES_EQ(expectedConstants, writer.constants);
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.back());
}
