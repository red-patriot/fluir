#include "compiler/backend/bytecode_generator.hpp"

#include <gtest/gtest.h>

#include "bytecode_assertions.hpp"
#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/frontend/type_checker.hpp"
#include "compiler/types/builtin_symbols.hpp"
#include "compiler/utility/pass.hpp"

namespace fa = fluir::asg;
namespace fc = fluir::code;
using namespace fc::value_literals;
using namespace fluir::literals_types;

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
  input.declarations.emplace_back(fa::FunctionDecl{
    .id = 3, .name = "foo", .statements = []() {
      fa::DataFlowGraph graph;
      graph.push_back(
        std::move(std::make_unique<fa::BinaryOp>(fluir::Operator::STAR,
                                                 std::make_shared<fa::Constant>(1.5, 3, fluir::FlowGraphLocation{}),
                                                 std::make_shared<fa::Constant>(2.5, 2, fluir::FlowGraphLocation{}),
                                                 1,
                                                 fluir::FlowGraphLocation{})));
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
                       graph.push_back(std::make_unique<fa::UnaryOp>(
                         fluir::Operator::MINUS,
                         std::make_shared<fa::Constant>(3.456, 3, fluir::FlowGraphLocation{}),
                         1,
                         fluir::FlowGraphLocation{}));
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
  auto shared = std::make_shared<fa::BinaryOp>(
    fluir::Operator::SLASH,
    std::make_shared<fa::UnaryOp>(fluir::Operator::MINUS,
                                  std::make_shared<fa::Constant>(3.5, 5, fluir::FlowGraphLocation{}),
                                  2,
                                  fluir::FlowGraphLocation{}),
    std::make_shared<fa::Constant>(4.4, 6, fluir::FlowGraphLocation{}),
    4,
    fluir::FlowGraphLocation{});
  fa::ASG input;
  input.declarations.emplace_back(fa::FunctionDecl{
    .id = 3, .name = "bar", .statements = [&]() {
      fa::DataFlowGraph graph;
      graph.push_back(
        std::make_unique<fa::BinaryOp>(fluir::Operator::PLUS,
                                       std::make_shared<fa::Constant>(100.0, 3, fluir::FlowGraphLocation{}),
                                       shared,
                                       1,
                                       fluir::FlowGraphLocation{}));
      graph.push_back(std::make_unique<fa::UnaryOp>(fluir::Operator::MINUS, shared, 7, fluir::FlowGraphLocation{}));
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

TEST(TestBytecodeGenerator, GeneratesIntConstants) {
  fa::ASG input;
  fluir::Context ctx{.symbolTable = fluir::types::buildSymbolTable()};
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "ints", .statements = {}};
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<I8>(8), 1, fluir::FlowGraphLocation{})));
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<I16>(16), 2, fluir::FlowGraphLocation{})));
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<I32>(32), 3, fluir::FlowGraphLocation{})));
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<I64>(64), 4, fluir::FlowGraphLocation{})));

    return fluir::checkTypes(ctx, std::move(decl)).value();
  }());

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 0, .patch = 0, .entryOffset = 0},
                        .chunks = {fc::Chunk{.name = "ints",
                                             .code =
                                               {
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
                                                 fc::Instruction::EXIT,
                                               },
                                             .constants = {8_i8, 16_i16, 32_i32, 64_i64}}}};

  auto actual = fluir::addContext(std::move(ctx), std::move(input)) | fluir::generateCode;

  EXPECT_FALSE(actual.ctx.diagnostics.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.data.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.data.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.data.value().chunks.at(0));
}

TEST(TestBytecodeGenerator, GeneratesUintConstants) {
  fa::ASG input;
  fluir::Context ctx{.symbolTable = fluir::types::buildSymbolTable()};
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "ints", .statements = {}};
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<U8>(8), 1, fluir::FlowGraphLocation{})));
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<U16>(16), 2, fluir::FlowGraphLocation{})));
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<U32>(32), 3, fluir::FlowGraphLocation{})));
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<U64>(64), 4, fluir::FlowGraphLocation{})));

    return fluir::checkTypes(ctx, std::move(decl)).value();
  }());

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 0, .patch = 0, .entryOffset = 0},
                        .chunks = {fc::Chunk{.name = "ints",
                                             .code =
                                               {
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
                                                 fc::Instruction::EXIT,
                                               },
                                             .constants = {8_u8, 16_u16, 32_u32, 64_u64}}}};

  auto actual = fluir::addContext(std::move(ctx), std::move(input)) | fluir::generateCode;

  EXPECT_FALSE(actual.ctx.diagnostics.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.data.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.data.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.data.value().chunks.at(0));
}

TEST(TestBytecodeGenerator, GeneratesIntBinaryExpression) {
  fa::ASG input;
  fluir::Context ctx{.symbolTable = fluir::types::buildSymbolTable()};
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "ints", .statements = {}};
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::PLUS,
      std::make_shared<fa::Constant>(static_cast<I32>(8), 1, fluir::FlowGraphLocation{}),
      std::make_shared<fa::Constant>(static_cast<I32>(16), 2, fluir::FlowGraphLocation{}),
      3,
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::MINUS,
      std::make_shared<fa::Constant>(static_cast<I32>(28), 1, fluir::FlowGraphLocation{}),
      std::make_shared<fa::Constant>(static_cast<I32>(16), 2, fluir::FlowGraphLocation{}),
      3,
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::STAR,
      std::make_shared<fa::Constant>(static_cast<I32>(8), 1, fluir::FlowGraphLocation{}),
      std::make_shared<fa::Constant>(static_cast<I32>(16), 2, fluir::FlowGraphLocation{}),
      3,
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::SLASH,
      std::make_shared<fa::Constant>(static_cast<I32>(8), 1, fluir::FlowGraphLocation{}),
      std::make_shared<fa::Constant>(static_cast<I32>(16), 2, fluir::FlowGraphLocation{}),
      3,
      fluir::FlowGraphLocation{})));

    return fluir::checkTypes(ctx, std::move(decl)).value();
  }());

  fc::ByteCode expected{
    .header = {.filetype = '\0', .major = 0, .minor = 0, .patch = 0, .entryOffset = 0},
    .chunks = {fc::Chunk{
      .name = "ints",
      .code =
        {
          fc::Instruction::PUSH, 0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::I64_ADD, fc::Instruction::POP,
          fc::Instruction::PUSH, 0x02, fc::Instruction::PUSH, 0x01, fc::Instruction::I64_SUB, fc::Instruction::POP,
          fc::Instruction::PUSH, 0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::I64_MUL, fc::Instruction::POP,
          fc::Instruction::PUSH, 0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::I64_DIV, fc::Instruction::POP,
          fc::Instruction::EXIT,
        },
      .constants = {8_i32, 16_i32, 28_i32}}}};

  auto actual = fluir::addContext(std::move(ctx), std::move(input)) | fluir::generateCode;

  EXPECT_FALSE(actual.ctx.diagnostics.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.data.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.data.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.data.value().chunks.at(0));
}

TEST(TestBytecodeGenerator, GeneratesUintBinaryExpression) {
  fa::ASG input;
  fluir::Context ctx{.symbolTable = fluir::types::buildSymbolTable()};
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "ints", .statements = {}};
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::PLUS,
      std::make_shared<fa::Constant>(static_cast<U32>(8), 1, fluir::FlowGraphLocation{}),
      std::make_shared<fa::Constant>(static_cast<U32>(16), 2, fluir::FlowGraphLocation{}),
      3,
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::MINUS,
      std::make_shared<fa::Constant>(static_cast<U32>(28), 1, fluir::FlowGraphLocation{}),
      std::make_shared<fa::Constant>(static_cast<U32>(16), 2, fluir::FlowGraphLocation{}),
      3,
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::STAR,
      std::make_shared<fa::Constant>(static_cast<U32>(8), 1, fluir::FlowGraphLocation{}),
      std::make_shared<fa::Constant>(static_cast<U32>(16), 2, fluir::FlowGraphLocation{}),
      3,
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::SLASH,
      std::make_shared<fa::Constant>(static_cast<U32>(8), 1, fluir::FlowGraphLocation{}),
      std::make_shared<fa::Constant>(static_cast<U32>(16), 2, fluir::FlowGraphLocation{}),
      3,
      fluir::FlowGraphLocation{})));

    return fluir::checkTypes(ctx, std::move(decl)).value();
  }());

  fc::ByteCode expected{
    .header = {.filetype = '\0', .major = 0, .minor = 0, .patch = 0, .entryOffset = 0},
    .chunks = {fc::Chunk{
      .name = "ints",
      .code =
        {
          fc::Instruction::PUSH, 0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::U64_ADD, fc::Instruction::POP,
          fc::Instruction::PUSH, 0x02, fc::Instruction::PUSH, 0x01, fc::Instruction::U64_SUB, fc::Instruction::POP,
          fc::Instruction::PUSH, 0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::U64_MUL, fc::Instruction::POP,
          fc::Instruction::PUSH, 0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::U64_DIV, fc::Instruction::POP,
          fc::Instruction::EXIT,
        },
      .constants = {8_u32, 16_u32, 28_u32}}}};

  auto actual = fluir::addContext(std::move(ctx), std::move(input)) | fluir::generateCode;

  EXPECT_FALSE(actual.ctx.diagnostics.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.data.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.data.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.data.value().chunks.at(0));
}
