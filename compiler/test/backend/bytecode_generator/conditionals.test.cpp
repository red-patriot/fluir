#include <gtest/gtest.h>

#include "ast_builders.hpp"
#include "bytecode/primitives.hpp"
#include "fixture.hpp"

namespace fc = fluir::code;
using namespace fluir::test;

TEST_F(TestBytecodeGenerator, GeneratesEmptyConditionalJumps) {
  fc::Chunk expectedChunk{
    .name = "cond",
    .code =
      {
        fc::RESERVE,
        0x1,
        fc::Instruction::PUSH,
        0x00,
        fc::SET_VAL,
        0x0,
        fc::GET_VAL,
        0x0,
        fc::Instruction::JUMP_IF_FALSE,
        0x0,
        0x0,
        0x0,
        0x5,
        fc::Instruction::JUMP_BY,
        0x0,
        0x0,
        0x0,
        0x0,
        fc::Instruction::MULTIPOP,
        0x1,
        fc::Instruction::RETURN,
      },
  };

  auto writer = generate(program(function(3, "cond", conditional({3, 1}, constant(true, {3, 2}), {}, {}, {}, {}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesConditionalWithBody) {
  // Constants: true, 7, 1, 3
  fc::Chunk expectedChunk{
    .name = "cond",
    .code =
      {
        fc::RESERVE,
        0x1,
        fc::Instruction::PUSH,
        0x00,
        fc::SET_VAL,
        0x0,
        fc::GET_VAL,
        0x0,
        fc::Instruction::JUMP_IF_FALSE,
        0x0,
        0x0,
        0x0,
        0xB,
        // THEN
        fc::Instruction::PUSH,
        0x1,
        fc::Instruction::PUSH,
        0x2,
        fc::Instruction::I64_ADD,
        fc::Instruction::POP,
        fc::Instruction::JUMP_BY,
        0x0,
        0x0,
        0x0,
        0x6,
        // ELSE
        fc::Instruction::PUSH,
        0x3,
        fc::Instruction::PUSH,
        0x2,
        fc::Instruction::I64_SUB,
        fc::Instruction::POP,
        fc::MULTIPOP,
        0x1,
        fc::Instruction::RETURN,
      },
  };

  auto writer = generate(program(function(
    3,
    "cond",
    conditional({3, 1},
                constant(true, {3, 2}),
                {},
                {},
                sinks(binary(fluir::Operator::PLUS, constant(7, {3, 0, 1}), constant(1, {3, 0, 2}), {3, 0, 3})),
                sinks(binary(fluir::Operator::MINUS, constant(3, {3, 1, 5}), constant(1, {3, 1, 4}), {3, 1, 6}))))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesConditionalWithInputs) {
  // Constants: 1, 3, true, 7
  fc::Chunk expectedChunk{
    .name = "cond",
    .code =
      {
        fc::RESERVE,
        0x3,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::PUSH,
        0x02,
        fc::SET_VAL,
        0x0,  // condition
        fc::SET_VAL,
        0x1,  // input 1
        fc::SET_VAL,
        0x2,  // input 0
        fc::GET_VAL,
        0x0,
        fc::Instruction::JUMP_IF_FALSE,
        0x0,
        0x0,
        0x0,
        0xB,
        // THEN
        fc::Instruction::PUSH,
        0x3,
        fc::Instruction::GET_VAL,
        0x2,  // input 0
        fc::Instruction::I64_ADD,
        fc::POP,
        fc::Instruction::JUMP_BY,
        0x0,
        0x0,
        0x0,
        0x6,
        // ELSE
        fc::Instruction::GET_VAL,
        0x2,  // input 0
        fc::Instruction::GET_VAL,
        0x1,  // input 1
        fc::Instruction::I64_SUB,
        fc::POP,
        fc::MULTIPOP,
        0x3,
        fc::Instruction::RETURN,
      },
  };

  auto writer = generate(
    program(function(3,
                     "cond",
                     conditional({3, 1},
                                 constant(true, {3, 2}),
                                 inputs(scopeInput(constant(1, {3, 3}), 14), scopeInput(constant(3, {3, 3}), 15)),
                                 {},
                                 sinks(binary(fluir::Operator::PLUS,
                                              constant(7, {3, 0, 1}),
                                              localRead(14, {3, 0}),  // 1
                                              {3, 0, 3})),
                                 sinks(binary(fluir::Operator::MINUS,
                                              localRead(14, {3, 1}),  // 1
                                              localRead(15, {3, 1}),  // 3
                                              {3, 1, 6}))))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesConditionalWithOutput) {
  // Constants: 1, true, 13
  fc::Chunk expectedChunk{
    .name = "cond",
    .code =
      {
        fc::RESERVE,
        0x2,
        fc::Instruction::PUSH,
        0x0,
        fc::Instruction::PUSH,
        0x1,
        fc::SET_VAL,
        0x0,  // condition
        fc::SET_VAL,
        0x1,  // input 0
        fc::GET_VAL,
        0x0,
        fc::Instruction::JUMP_IF_FALSE,
        0x0,
        0x0,
        0x0,
        0xA,
        // THEN
        fc::Instruction::PUSH,
        0x2,
        fc::Instruction::GET_VAL,
        0x1,  // input 0
        fc::Instruction::I64_ADD,
        fc::Instruction::JUMP_BY,
        0x0,
        0x0,
        0x0,
        0x2,
        // ELSE
        fc::Instruction::PUSH,
        0x3,
        fc::POP,
        fc::Instruction::MULTIPOP,
        0x2,
        fc::Instruction::RETURN,
      },
  };

  auto writer = generate(program(function(3,
                                          "cond",
                                          conditional({3, 1},
                                                      constant(true, {3, 2}),
                                                      inputs(scopeInput(constant(1, {3, 3}), 14)),
                                                      {12},
                                                      // THEN
                                                      output(binary(fluir::Operator::PLUS,
                                                                    constant(7, {3, 0, 1}),
                                                                    localRead(14, {3, 0}),  // 1
                                                                    {3, 0, 3})),
                                                      // ELSE
                                                      output(constant(13, {3, 1, 19}))))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesConditionalWithMultipleOutputs) {
  // Constants: 1, true, 7, false, 13
  fc::Chunk expectedChunk{
    .name = "cond",
    .code =
      {
        fc::Instruction::RESERVE,
        0x2,
        fc::Instruction::PUSH,
        0x0,
        fc::Instruction::PUSH,
        0x1,
        fc::SET_VAL,
        0x0,  // condition
        fc::SET_VAL,
        0x1,  // input 0
        fc::GET_VAL,
        0x0,
        fc::Instruction::JUMP_IF_FALSE,
        0x0,
        0x0,
        0x0,
        0xC,
        // THEN
        fc::Instruction::PUSH,
        0x2,
        fc::Instruction::GET_VAL,
        0x1,  // input 0
        fc::Instruction::I64_ADD,
        fc::Instruction::PUSH,
        0x3,
        fc::Instruction::JUMP_BY,
        0x0,
        0x0,
        0x0,
        0x4,
        // ELSE
        fc::Instruction::PUSH,
        0x4,
        fc::Instruction::PUSH,
        0x1,
        fc::Instruction::MULTIPOP,
        0x2,
        fc::Instruction::MULTIPOP,
        0x2,
        fc::Instruction::RETURN,
      },
  };

  auto writer = generate(program(
    function(3,
             "cond",
             conditional({3, 1},
                         constant(true, {3, 2}),
                         inputs(scopeInput(constant(1, {3, 3}), 14)),
                         {12, 13},
                         // THEN
                         output(bundle({3, 1, 1},
                                       deps(binary(fluir::Operator::PLUS,
                                                   constant(7, {3, 0, 1}),
                                                   localRead(14, {3, 0, 2}),  // 1
                                                   {3, 1, 3}),
                                            constant(false, {3, 0, 4})))),
                         // ELSE
                         output(bundle({3, 1, 2}, deps(constant(13, {3, 1, 19}), constant(true, {3, 1, 21}))))))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesUpstreamNodesOfConditionalWithMultipleOutputs) {
  // Constants: 1, true, 7, false, 13, "print"
  fc::Chunk expectedChunk{
    .name = "cond",
    .code =
      {
        fc::Instruction::RESERVE,
        0x2,
        fc::Instruction::PUSH,
        0x0,
        fc::Instruction::PUSH,
        0x1,
        fc::SET_VAL,
        0x0,  // condition
        fc::SET_VAL,
        0x1,  // input 0
        fc::GET_VAL,
        0x0,
        fc::Instruction::JUMP_IF_FALSE,
        0x0,
        0x0,
        0x0,
        0xC,
        // THEN
        fc::Instruction::PUSH,
        0x2,
        fc::Instruction::GET_VAL,
        0x1,  // input 0
        fc::Instruction::I64_ADD,
        fc::Instruction::PUSH,
        0x3,
        fc::Instruction::JUMP_BY,
        0x0,
        0x0,
        0x0,
        0x4,
        // ELSE
        fc::Instruction::PUSH,
        0x4,
        fc::Instruction::PUSH,
        0x1,
        fc::Instruction::SET_VAL,
        0x1,
        fc::Instruction::SET_VAL,
        0x0,
        fc::Instruction::GET_VAL,
        0x1,
        fc::DYN_CALL,
        0x0,
        0x0,
        0x0,
        0x5,
        fc::Instruction::MULTIPOP,
        0x2,
        fc::Instruction::RETURN,
      },
  };

  auto writer = generate(program(function(
    3,
    "cond",
    call(
      "print",
      deps(unbundle({3, 1},
                    conditional({3, 1},
                                constant(true, {3, 2}),
                                inputs(scopeInput(constant(1, {3, 3}), 14)),
                                {12, 13},
                                // THEN
                                output(bundle({3, 1, 1},
                                              deps(binary(fluir::Operator::PLUS,
                                                          constant(7, {3, 0, 1}),
                                                          localRead(14, {3, 0, 2}),  // 1
                                                          {3, 1, 3}),
                                                   constant(false, {3, 0, 4})))),
                                // ELSE
                                output(bundle({3, 1, 2}, deps(constant(13, {3, 1, 19}), constant(true, {3, 1, 21}))))),
                    1)),
      {3, 26}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesMultipleUpstreamNodesOfConditionalWithMultipleOutputs) {
  // Constants: 1, true, 7, false, 13, "print"
  fc::Chunk expectedChunk{
    .name = "cond",
    .code =
      {
        fc::Instruction::RESERVE,
        0x2,
        fc::Instruction::PUSH,
        0x0,
        fc::Instruction::PUSH,
        0x1,
        fc::SET_VAL,
        0x0,  // condition
        fc::SET_VAL,
        0x1,  // input 0
        fc::GET_VAL,
        0x0,
        fc::Instruction::JUMP_IF_FALSE,
        0x0,
        0x0,
        0x0,
        0xC,
        // THEN
        fc::Instruction::PUSH,
        0x2,
        fc::Instruction::GET_VAL,
        0x1,  // input 0
        fc::Instruction::I64_ADD,
        fc::Instruction::PUSH,
        0x3,
        fc::Instruction::JUMP_BY,
        0x0,
        0x0,
        0x0,
        0x4,
        // ELSE
        fc::Instruction::PUSH,
        0x4,
        fc::Instruction::PUSH,
        0x1,
        fc::Instruction::SET_VAL,
        0x1,
        fc::Instruction::SET_VAL,
        0x0,
        fc::Instruction::GET_VAL,
        0x1,
        fc::DYN_CALL,
        0x0,
        0x0,
        0x0,
        0x5,
        fc::Instruction::GET_VAL,
        0x0,
        fc::DYN_CALL,
        0x0,
        0x0,
        0x0,
        0x5,
        fc::Instruction::MULTIPOP,
        0x2,
        fc::Instruction::RETURN,
      },
  };

  auto writer = generate(program(function(
    3,
    "cond",
    localWrite(conditional({3, 1},
                           constant(true, {3, 2}),
                           inputs(scopeInput(constant(1, {3, 3}), 14)),
                           {12, 13},
                           // THEN
                           output(bundle({3, 1, 1},
                                         deps(binary(fluir::Operator::PLUS,
                                                     constant(7, {3, 0, 1}),
                                                     localRead(14, {3, 0, 2}),  // 1
                                                     {3, 1, 3}),
                                              constant(false, {3, 0, 4})))),
                           // ELSE
                           output(bundle({3, 1, 2}, deps(constant(13, {3, 1, 19}), constant(true, {3, 1, 21})))))),
    call("print", deps(unbundle({3, 4}, localRead(1, {3, 4}), 1)), {3, 4}),
    call("print", deps(unbundle({3, 5}, localRead(1, {3, 5}), 0)), {3, 5}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

// There was a bug where using multiple outputs with no inputs would produce incorrect output because it saved the
// incorrect number of slots. This test replicates the bug
TEST_F(TestBytecodeGenerator, MultiOutputNoInputBug) {
  // Constants: true, 1, 2, 3, 4, "print"
  fc::Chunk expectedChunk{
    .name = "cond",
    .code =
      {
        fc::Instruction::RESERVE,
        0x2,
        fc::Instruction::PUSH,
        0x0,
        fc::SET_VAL,
        0x0,  // condition
        fc::GET_VAL,
        0x0,
        fc::Instruction::JUMP_IF_FALSE,
        0x0,
        0x0,
        0x0,
        0x9,
        // THEN
        fc::Instruction::PUSH,
        0x1,
        fc::Instruction::PUSH,
        0x2,
        fc::Instruction::JUMP_BY,
        0x0,
        0x0,
        0x0,
        0x4,
        // ELSE
        fc::Instruction::PUSH,
        0x3,
        fc::Instruction::PUSH,
        0x4,
        fc::Instruction::SET_VAL,
        0x1,
        fc::Instruction::SET_VAL,
        0x0,
        fc::Instruction::GET_VAL,
        0x1,
        fc::DYN_CALL,
        0x0,
        0x0,
        0x0,
        0x5,
        fc::Instruction::GET_VAL,
        0x0,
        fc::DYN_CALL,
        0x0,
        0x0,
        0x0,
        0x5,
        fc::Instruction::MULTIPOP,
        0x2,
        fc::Instruction::RETURN,
      },
  };

  auto writer = generate(program(
    function(3,
             "cond",
             localWrite(conditional({3, 1},
                                    constant(true, {3, 2}),
                                    inputs(),
                                    {},
                                    // THEN
                                    output(bundle({3, 1, 1}, deps(constant(1, {3, 0, 5}), constant(2, {3, 0, 4})))),
                                    // ELSE
                                    output(bundle({3, 1, 2}, deps(constant(3, {3, 1, 19}), constant(4, {3, 1, 21})))))),
             call("print", deps(unbundle({3, 4}, localRead(1, {3, 4}), 1)), {3, 4}),
             call("print", deps(unbundle({3, 5}, localRead(1, {3, 5}), 0)), {3, 5}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}
