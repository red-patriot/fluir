#include "compiler/backend/bytecode_generator.hpp"

#include <gtest/gtest.h>

#include "bytecode_assertions.hpp"

namespace fa = fluir::asg;
namespace fc = fluir::code;

TEST(TestBytecodeGenerator, GeneratesEmptyFunction) {
  fa::ASG input{
      .declarations =
          {fa::FunctionDecl{
              .id = 3,
              .name = "main",
              .statements = {}}}};

  fc::ByteCode expected{
      .header = {.filetype = '\0',
                 .major = 0,
                 .minor = 0,
                 .patch = 0,
                 .entryOffset = 0},
      .chunks = {fc::Chunk{.name = "main", .code = {fc::Instruction::EXIT}, .constants = {}}}};

  auto actual = fluir::generateCode(input);

  EXPECT_FALSE(actual.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST(TestBytecodeGenerator, GeneratesEmptyFunctions) {
  fa::ASG input{
      .declarations =
          {fa::FunctionDecl{
               .id = 3,
               .name = "main",
               .statements = {}},
           fa::FunctionDecl{
               .id = 2,
               .name = "foo",
               .statements = {}}}};

  fc::ByteCode expected{
      .header = {.filetype = '\0',
                 .major = 0,
                 .minor = 0,
                 .patch = 0,
                 .entryOffset = 0},
      .chunks = {fc::Chunk{.name = "main",
                           .code = {fc::Instruction::EXIT},
                           .constants = {}},
                 fc::Chunk{.name = "foo",
                           .code = {fc::Instruction::EXIT},
                           .constants = {}}}};

  auto actual = fluir::generateCode(input);

  EXPECT_FALSE(actual.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  for (int i = 0; i != expected.chunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expected.chunks.at(i), actual.value().chunks.at(i));
  }
}

TEST(TestBytecodeGenerator, GeneratesSimpleBinaryExpression) {
  fa::ASG input{
      .declarations =
          {fa::FunctionDecl{
              .id = 3,
              .name = "foo",
              .statements = {
                  fa::BinaryOp{
                      1,
                      {},
                      fluir::Operator::STAR,
                      std::make_shared<fa::Node>(fa::ConstantFP{3, {}, 1.5}),
                      std::make_shared<fa::Node>(fa::ConstantFP{2, {}, 2.5})}}}}};

  fc::ByteCode expected{
      .header = {
          .filetype = '\0',
          .major = 0,
          .minor = 0,
          .patch = 0,
          .entryOffset = 0},
      .chunks = {fc::Chunk{.name = "foo", .code = {
                                              fc::Instruction::PUSH_FP,
                                              0x00,
                                              fc::Instruction::PUSH_FP,
                                              0x01,
                                              fc::Instruction::FP_MULTIPLY,
                                              fc::Instruction::POP,
                                              fc::Instruction::EXIT,
                                          },
                           .constants = {1.5, 2.5}}}};

  auto actual = fluir::generateCode(input);

  EXPECT_FALSE(actual.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}
