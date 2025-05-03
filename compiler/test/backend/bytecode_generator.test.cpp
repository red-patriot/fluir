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

TEST(TestBytecodeGenerator, GeneratesSimpleUnaryExpression) {
  fa::ASG input{
      .declarations =
          {fa::FunctionDecl{
              .id = 3,
              .name = "bar",
              .statements = {
                  fa::UnaryOp{
                      1,
                      {},
                      fluir::Operator::MINUS,
                      std::make_shared<fa::Node>(fa::ConstantFP{3, {}, 3.456})}}}}};

  fc::ByteCode expected{
      .header = {
          .filetype = '\0',
          .major = 0,
          .minor = 0,
          .patch = 0,
          .entryOffset = 0},
      .chunks = {fc::Chunk{.name = "bar", .code = {
                                              fc::Instruction::PUSH_FP,
                                              0x00,
                                              fc::Instruction::FP_NEGATE,
                                              fc::Instruction::POP,
                                              fc::Instruction::EXIT,
                                          },
                           .constants = {3.456}}}};

  auto actual = fluir::generateCode(input);

  EXPECT_FALSE(actual.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST(TestBytecodeGenerator, GeneratesExpressionWithSharedNodes) {
  auto shared = std::make_shared<fa::Node>(
      fa::BinaryOp{
          4,
          {},
          fluir::Operator::SLASH,
          std::make_shared<fa::Node>(
              fa::UnaryOp{
                  2,
                  {},
                  fluir::Operator::MINUS,
                  std::make_shared<fa::Node>(fa::ConstantFP{5, {}, 3.5})}),
          std::make_shared<fa::Node>(
              fa::ConstantFP{6, {}, -4.4})});
  fa::ASG input{
      .declarations =
          {fa::FunctionDecl{
              .id = 3,
              .name = "bar",
              .statements = {
                  fa::BinaryOp{
                      1,
                      {},
                      fluir::Operator::PLUS,
                      std::make_shared<fa::Node>(
                          fa::ConstantFP{3, {}, 100.0}),
                      shared},
                  fa::UnaryOp{
                      7,
                      {},
                      fluir::Operator::MINUS,
                      shared}}}}};

  fc::ByteCode expected{
      .header = {
          .filetype = '\0',
          .major = 0,
          .minor = 0,
          .patch = 0,
          .entryOffset = 0},
      .chunks = {fc::Chunk{.name = "bar", .code = {
                                              fc::PUSH_FP,
                                              0x00,
                                              fc::PUSH_FP,
                                              0x01,
                                              fc::FP_NEGATE,
                                              fc::PUSH_FP,
                                              0x02,
                                              fc::FP_DIVIDE,
                                              fc::FP_ADD,
                                              fc::POP,
                                              fc::PUSH_FP,
                                              0x01,
                                              fc::FP_NEGATE,
                                              fc::PUSH_FP,
                                              0x02,
                                              fc::FP_DIVIDE,
                                              fc::FP_NEGATE,
                                              fc::POP,
                                              fc::Instruction::EXIT,
                                          },
                           .constants = {100.0, 3.5, -4.4}}}};

  auto actual = fluir::generateCode(input);

  EXPECT_FALSE(actual.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}
