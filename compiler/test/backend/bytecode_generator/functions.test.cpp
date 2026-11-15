#include <gtest/gtest.h>

#include "ast_builders.hpp"
#include "bytecode/primitives.hpp"
#include "fixture.hpp"

namespace fa = fluir::ast;
namespace fc = fluir::code;
using namespace fluir::literals_types;
using namespace fluir::test;
using namespace std::string_literals;

TEST_F(TestBytecodeGenerator, GeneratesEmptyFunction) {
  fc::Chunk expectedChunk{.name = "main", .code = {fc::RESERVE, 0x0, fc::RETURN}};

  auto writer = generate(program(function(3, "main")));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesEmptyFunctions) {
  std::vector expectedChunks{fc::Chunk{.name = "main", .code = {fc::RESERVE, 0x0, fc::Instruction::RETURN}},
                             fc::Chunk{.name = "foo", .code = {fc::RESERVE, 0x0, fc::Instruction::RETURN}}};

  auto writer = generate(program(function(3, "main"), function(2, "foo")));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(expectedChunks.size(), writer.chunks.size());
  for (int i = 0; i != expectedChunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expectedChunks.at(i), writer.chunks.at(i));
  }
}

TEST_F(TestBytecodeGenerator, GeneratesFunctionCalls) {
  auto main = function(2, "main", call("add_nums", deps(constant(I32{2}, {2, 1}), constant(I32{3}, {2, 2})), {2, 3}));
  auto addNums =
    function(3,
             "add_nums",
             localWrite({3, 1}, binary(fluir::Operator::PLUS, localRead(3, {3, 4}), localRead(2, {3, 4}), {3, 4})));
  addNums.parameters.push_back(fa::FunctionDecl::Parameter{.id = 3, .name = "a", .typeName = "I32"});
  addNums.parameters.push_back(fa::FunctionDecl::Parameter{.id = 2, .name = "b", .typeName = "I32"});
  addNums.returnValue = fa::FunctionDecl::Return{.id = 1, .typeName = "I32"};

  std::vector expectedChunks{fc::Chunk{.name = "main",
                                       .code =
                                         {
                                           fc::RESERVE,
                                           0x0,
                                           fc::RESERVE,  // Reserve space for the return
                                           0x01,
                                           fc::PUSH,
                                           0x00,  // Arg 1
                                           fc::PUSH,
                                           0x01,  // Arg 2
                                           fc::CALL,
                                           0x00,
                                           0x00,
                                           0x00,
                                           0x01,
                                           fc::Instruction::POP,
                                           fc::Instruction::RETURN,
                                         }},
                             fc::Chunk{.name = "add_nums",
                                       .code =
                                         {
                                           fc::RESERVE,
                                           0x0,
                                           fc::GET_VAL,
                                           0x1,
                                           fc::GET_VAL,
                                           0x2,
                                           fc::I64_ADD,
                                           fc::SET_VAL,
                                           0x0,
                                           fc::MULTIPOP,
                                           0x2,
                                           fc::Instruction::RETURN,
                                         },
                                       .inCount = 2,
                                       .outCount = 1}};

  auto writer = generate(program(std::move(main), std::move(addNums)));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(expectedChunks.size(), writer.chunks.size());
  for (size_t i = 0; i != expectedChunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expectedChunks.at(i), writer.chunks.at(i));
  }
}

TEST_F(TestBytecodeGenerator, GeneratesFunctionCallWithNoReturn) {
  auto sink = function(2, "sink");
  sink.parameters.push_back(fa::FunctionDecl::Parameter{.id = 3, .name = "x", .typeName = "I32"});

  std::vector expectedChunks{fc::Chunk{.name = "main",
                                       .code =
                                         {
                                           fc::RESERVE,
                                           0x0,
                                           fc::PUSH,
                                           0x00,  // arg 7
                                           fc::CALL,
                                           0x00,
                                           0x00,
                                           0x00,
                                           0x01,
                                           // No return => no pop
                                           fc::Instruction::RETURN,
                                         }},
                             fc::Chunk{.name = "sink",
                                       .code =
                                         {
                                           fc::RESERVE,
                                           0x0,
                                           fc::MULTIPOP,
                                           0x01,
                                           fc::Instruction::RETURN,
                                         },
                                       .inCount = 1,
                                       .outCount = 0}};

  auto writer =
    generate(program(function(1, "main", call("sink", deps(constant(I32{7}, {1, 1})), {1, 2})), std::move(sink)));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(expectedChunks.size(), writer.chunks.size());
  for (int i = 0; i != static_cast<int>(expectedChunks.size()); ++i) {
    EXPECT_CHUNK_EQ(expectedChunks.at(i), writer.chunks.at(i));
  }
}

TEST_F(TestBytecodeGenerator, GeneratesFunctionCallWithNoArguments) {
  auto getVal = function(2, "get_val", localWrite({2, 1}, constant(12, {2, 2})));
  getVal.returnValue = fa::FunctionDecl::Return{.id = 1, .typeName = "I32"};

  std::vector expectedChunks{fc::Chunk{.name = "main",
                                       .code =
                                         {
                                           fc::RESERVE,
                                           0x0,
                                           fc::RESERVE,
                                           0x01,
                                           fc::CALL,
                                           0x00,
                                           0x00,
                                           0x00,
                                           0x01,
                                           fc::Instruction::POP,
                                           fc::Instruction::RETURN,
                                         }},
                             fc::Chunk{.name = "get_val",
                                       .code =
                                         {
                                           fc::RESERVE,
                                           0x0,
                                           fc::Instruction::PUSH,
                                           0x0,
                                           fc::Instruction::SET_VAL,
                                           0x0,
                                           fc::Instruction::RETURN,
                                         },
                                       .inCount = 0,
                                       .outCount = 1}};

  auto writer = generate(program(function(1, "main", call("get_val", {}, {1, 1})), std::move(getVal)));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(expectedChunks.size(), writer.chunks.size());
  for (int i = 0; i != static_cast<int>(expectedChunks.size()); ++i) {
    EXPECT_CHUNK_EQ(expectedChunks.at(i), writer.chunks.at(i));
  }
}

TEST_F(TestBytecodeGenerator, GeneratesMultipleFunctionCalls) {
  auto first = function(2, "first");
  first.returnValue = fa::FunctionDecl::Return{.id = 1, .typeName = "I32"};
  auto second = function(3, "second");
  second.returnValue = fa::FunctionDecl::Return{.id = 1, .typeName = "I32"};

  std::vector expectedChunks{
    fc::Chunk{.name = "main",
              .code =
                {
                  fc::RESERVE,
                  0x0,
                  fc::RESERVE,
                  0x01,
                  fc::CALL,
                  0x00,
                  0x00,
                  0x00,
                  0x01,
                  fc::POP,
                  fc::RESERVE,
                  0x01,
                  fc::CALL,
                  0x00,
                  0x00,
                  0x00,
                  0x02,
                  fc::POP,
                  fc::Instruction::RETURN,
                }},
    fc::Chunk{.name = "first", .code = {fc::RESERVE, 0x0, fc::Instruction::RETURN}, .inCount = 0, .outCount = 1},
    fc::Chunk{.name = "second", .code = {fc::RESERVE, 0x0, fc::Instruction::RETURN}, .inCount = 0, .outCount = 1}};

  auto writer = generate(program(
    function(1, "main", call("first", {}, {1, 1}), call("second", {}, {1, 2})), std::move(first), std::move(second)));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(expectedChunks.size(), writer.chunks.size());
  for (int i = 0; i != static_cast<int>(expectedChunks.size()); ++i) {
    EXPECT_CHUNK_EQ(expectedChunks.at(i), writer.chunks.at(i));
  }
}

TEST_F(TestBytecodeGenerator, GeneratesCalleeChunkForFunctionWithParameters) {
  auto add = function(1, "add");
  add.returnValue = fa::FunctionDecl::Return{.id = 1, .typeName = "I32"};
  add.parameters.push_back(fa::FunctionDecl::Parameter{.id = 2, .name = "a", .typeName = "I32"});
  add.parameters.push_back(fa::FunctionDecl::Parameter{.id = 3, .name = "b", .typeName = "I32"});

  std::vector expectedChunks{fc::Chunk{.name = "add",
                                       .code =
                                         {
                                           fc::RESERVE,
                                           0x0,
                                           fc::MULTIPOP,
                                           0x02,
                                           fc::Instruction::RETURN,
                                         },
                                       .inCount = 2,
                                       .outCount = 1},
                             fc::Chunk{.name = "main",
                                       .code = {
                                         fc::RESERVE,
                                         0x0,
                                         fc::RESERVE,
                                         0x01,
                                         fc::PUSH,
                                         0x00,  // arg 10
                                         fc::PUSH,
                                         0x01,  // arg 20
                                         fc::CALL,
                                         0x00,
                                         0x00,
                                         0x00,
                                         0x00,  // add is at index 0
                                         fc::Instruction::POP,
                                         fc::Instruction::RETURN,
                                       }}};

  auto writer = generate(
    program(std::move(add),
            function(2, "main", call("add", deps(constant(I32{10}, {2, 1}), constant(I32{20}, {2, 2})), {2, 3}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  ASSERT_EQ(expectedChunks.size(), writer.chunks.size());
  for (int i = 0; i != static_cast<int>(expectedChunks.size()); ++i) {
    EXPECT_CHUNK_EQ(expectedChunks.at(i), writer.chunks.at(i));
  }
}

TEST_F(TestBytecodeGenerator, GeneratesCodeForBuiltinFunctions) {
  fluir::be::ConstantsArray expectedConstants{12.5, "print"s};
  fc::Chunk expectedChunk{.name = "main",
                          .code = {
                            fc::RESERVE,
                            0x0,
                            fc::PUSH,
                            0x00,  // arg 12.5
                            fc::DYN_CALL,
                            0x00,
                            0x00,
                            0x00,
                            0x01,
                            fc::Instruction::RETURN,
                          }};

  auto writer = generate(program(function(1, "main", call("print", deps(constant(12.5, {1, 2})), {1, 1}))));

  EXPECT_BC_HEADER_EQ(expectedHeader(), writer.header);
  EXPECT_EQ(writer.constants, expectedConstants);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}
