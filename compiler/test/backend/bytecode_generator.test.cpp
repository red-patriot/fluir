#include "compiler/backend/bytecode_generator.hpp"

#include <gtest/gtest.h>

#include "bytecode_assertions.hpp"
#include "compiler/utility/pass.hpp"

namespace fa = fluir::asg;
namespace fc = fluir::code;
using namespace fc::value_literals;

TEST(TestBytecodeGenerator, GeneratesEmptyFunction) {
  fa::ASG input;
  input.declarations.emplace_back(fa::FunctionDecl{.id = 3, .name = "main", .statements = {}});

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 0, .patch = 0, .entryOffset = 0},
                        .chunks = {fc::Chunk{.name = "main", .code = {fc::Instruction::EXIT}, .constants = {}}}};

  auto [ctx, actual] = fluir::addContext(fluir::Context{}, std::move(input)) | fluir::generateCode;

  EXPECT_FALSE(ctx.diagnostics.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST(TestBytecodeGenerator, GeneratesEmptyFunctions) {
  fa::ASG input;
  input.declarations.emplace_back(fa::FunctionDecl{.id = 3, .name = "main", .statements = {}});
  input.declarations.emplace_back(fa::FunctionDecl{.id = 2, .name = "foo", .statements = {}});

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 0, .patch = 0, .entryOffset = 0},
                        .chunks = {fc::Chunk{.name = "main", .code = {fc::Instruction::EXIT}, .constants = {}},
                                   fc::Chunk{.name = "foo", .code = {fc::Instruction::EXIT}, .constants = {}}}};

  auto [ctx, actual] = fluir::addContext(fluir::Context{}, std::move(input)) | fluir::generateCode;

  EXPECT_FALSE(ctx.diagnostics.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  for (int i = 0; i != expected.chunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expected.chunks.at(i), actual.value().chunks.at(i));
  }
}

TEST(TestBytecodeGenerator, GeneratesSimpleBinaryExpression) {
  fa::ASG input;
  input.declarations.emplace_back(fa::FunctionDecl{.id = 3, .name = "foo", .statements = []() {
                                                     fa::DataFlowGraph graph;
                                                     graph.push_back(std::move(std::make_unique<fa::Node>(fa::BinaryOp{
                                                       1,
                                                       {},
                                                       fluir::Operator::STAR,
                                                       std::make_shared<fa::Node>(fa::ConstantFP{3, {}, 1.5}),
                                                       std::make_shared<fa::Node>(fa::ConstantFP{2, {}, 2.5})})));
                                                     return graph;
                                                   }()});

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 0, .patch = 0, .entryOffset = 0},
                        .chunks = {fc::Chunk{.name = "foo",
                                             .code =
                                               {
                                                 fc::Instruction::PUSH,
                                                 0x00,
                                                 fc::Instruction::PUSH,
                                                 0x01,
                                                 fc::Instruction::F64_MUL,
                                                 fc::Instruction::POP,
                                                 fc::Instruction::EXIT,
                                               },
                                             .constants = {1.5_f64, 2.5_f64}}}};

  auto [ctx, actual] = fluir::addContext(fluir::Context{}, std::move(input)) | fluir::generateCode;

  EXPECT_FALSE(ctx.diagnostics.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST(TestBytecodeGenerator, GeneratesSimpleUnaryExpression) {
  fa::ASG input;
  input.declarations.emplace_back(
    fa::FunctionDecl{.id = 3, .name = "bar", .statements = []() {
                       fa::DataFlowGraph graph;
                       graph.push_back(std::make_unique<fa::Node>(fa::UnaryOp{
                         1, {}, fluir::Operator::MINUS, std::make_shared<fa::Node>(fa::ConstantFP{3, {}, 3.456})}));
                       return graph;
                     }()});

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 0, .patch = 0, .entryOffset = 0},
                        .chunks = {fc::Chunk{.name = "bar",
                                             .code =
                                               {
                                                 fc::Instruction::PUSH,
                                                 0x00,
                                                 fc::Instruction::F64_NEG,
                                                 fc::Instruction::POP,
                                                 fc::Instruction::EXIT,
                                               },
                                             .constants = {3.456_f64}}}};

  auto [ctx, actual] = fluir::addContext(fluir::Context{}, std::move(input)) | fluir::generateCode;

  EXPECT_FALSE(ctx.diagnostics.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST(TestBytecodeGenerator, GeneratesExpressionWithSharedNodes) {
  auto shared = std::make_shared<fa::Node>(
    fa::BinaryOp{4,
                 {},
                 fluir::Operator::SLASH,
                 std::make_shared<fa::Node>(
                   fa::UnaryOp{2, {}, fluir::Operator::MINUS, std::make_shared<fa::Node>(fa::ConstantFP{5, {}, 3.5})}),
                 std::make_shared<fa::Node>(fa::ConstantFP{6, {}, 4.4})});
  fa::ASG input;
  input.declarations.emplace_back(fa::FunctionDecl{
    .id = 3, .name = "bar", .statements = [&]() {
      fa::DataFlowGraph graph;
      graph.push_back(std::make_unique<fa::Node>(
        fa::BinaryOp{1, {}, fluir::Operator::PLUS, std::make_shared<fa::Node>(fa::ConstantFP{3, {}, 100.0}), shared}));
      graph.push_back(std::make_unique<fa::Node>(fa::UnaryOp{7, {}, fluir::Operator::MINUS, shared}));
      return graph;
    }()});

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 0, .patch = 0, .entryOffset = 0},
                        .chunks = {fc::Chunk{.name = "bar",
                                             .code =
                                               {
                                                 fc::PUSH,
                                                 0x00,
                                                 fc::PUSH,
                                                 0x01,
                                                 fc::F64_NEG,
                                                 fc::PUSH,
                                                 0x02,
                                                 fc::F64_DIV,
                                                 fc::F64_ADD,
                                                 fc::POP,
                                                 fc::PUSH,
                                                 0x01,
                                                 fc::F64_NEG,
                                                 fc::PUSH,
                                                 0x02,
                                                 fc::F64_DIV,
                                                 fc::F64_NEG,
                                                 fc::POP,
                                                 fc::Instruction::EXIT,
                                               },
                                             .constants = {100.0_f64, 3.5_f64, 4.4_f64}}}};

  auto [ctx, actual] = fluir::addContext(fluir::Context{}, std::move(input)) | fluir::generateCode;

  EXPECT_FALSE(ctx.diagnostics.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}
