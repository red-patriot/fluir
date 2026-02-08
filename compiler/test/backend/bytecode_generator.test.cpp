#include "compiler/backend/bytecode_generator.hpp"

#include <gtest/gtest.h>

#include "bytecode_assertions.hpp"
#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/frontend/type_checker.hpp"
#include "compiler/types/builtin_symbols.hpp"
#include "test_diagnostic_sink.hpp"

namespace fa = fluir::ast;
namespace fc = fluir::code;
namespace ft = fluir::types;
using namespace fc::value_literals;
using namespace fluir::literals_types;

using fluir::FullID;

class TestBytecodeGenerator : public ::testing::Test {
 public:
  fluir::test::TestDiagnosticSink sink_;
  fluir::Context ctx_{.diagnosticSink = sink_,
                      .symbolTable = fluir::types::buildSymbolTable(),
                      .version = fluir::Version{.major = 0, .minor = 1, .patch = 3}};
};

TEST_F(TestBytecodeGenerator, GeneratesEmptyFunction) {
  fa::AST input;
  input.declarations.emplace_back(fa::FunctionDecl{.id = 3, .name = "main", .statements = {}});

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0},
                        .chunks = {fc::Chunk{.name = "main", .code = {fc::Instruction::EXIT}, .constants = {}}}};

  auto actual = fluir::generateCode(ctx_, input);

  EXPECT_FALSE(sink_.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST_F(TestBytecodeGenerator, GeneratesEmptyFunctions) {
  fa::AST input;
  input.declarations.emplace_back(fa::FunctionDecl{.id = 3, .name = "main", .statements = {}});
  input.declarations.emplace_back(fa::FunctionDecl{.id = 2, .name = "foo", .statements = {}});

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0},
                        .chunks = {fc::Chunk{.name = "main", .code = {fc::Instruction::EXIT}, .constants = {}},
                                   fc::Chunk{.name = "foo", .code = {fc::Instruction::EXIT}, .constants = {}}}};

  auto actual = fluir::generateCode(ctx_, input);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  for (int i = 0; i != expected.chunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expected.chunks.at(i), actual.value().chunks.at(i));
  }
}

TEST_F(TestBytecodeGenerator, GeneratesSimpleBinaryExpression) {
  fa::AST input;
  input.declarations.emplace_back(
    fa::FunctionDecl{.id = 3, .name = "foo", .statements = []() {
                       fa::DataFlowGraph graph;
                       graph.push_back(std::move(std::make_unique<fa::BinaryOp>(
                         fluir::Operator::STAR,
                         fa::createDependency<fa::Constant>(1.5, FullID{3, 3}, fluir::FlowGraphLocation{}),
                         fa::createDependency<fa::Constant>(2.5, FullID{3, 2}, fluir::FlowGraphLocation{}),
                         FullID{3, 1},
                         fluir::FlowGraphLocation{})));
                       return graph;
                     }()});

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0},
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

  auto actual = fluir::generateCode(ctx_, input);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST_F(TestBytecodeGenerator, GeneratesSimpleUnaryExpression) {
  fa::AST input;
  input.declarations.emplace_back(
    fa::FunctionDecl{.id = 3, .name = "bar", .statements = []() {
                       fa::DataFlowGraph graph;
                       graph.push_back(std::make_unique<fa::UnaryOp>(
                         fluir::Operator::MINUS,
                         fa::createDependency<fa::Constant>(3.456, FullID{3, 3}, fluir::FlowGraphLocation{}),
                         FullID{3, 1},
                         fluir::FlowGraphLocation{}));
                       return graph;
                     }()});

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0},
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

  auto actual = fluir::generateCode(ctx_, input);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST_F(TestBytecodeGenerator, GeneratesExpressionWithSharedNodes) {
  const FullID SHARED_ID{3, 4};
  fa::AST input;
  input.declarations.push_back([&]() { return fa::Declaration{.id = 3, .name = "bar", .statements = {}}; }());
  auto& decl = input.declarations.back();
  {
    auto shared = fa::createDependency<fa::LocalWrite>(
      fa::createDependency<fa::BinaryOp>(
        fluir::Operator::SLASH,
        fa::createDependency<fa::UnaryOp>(
          fluir::Operator::MINUS,
          fa::createDependency<fa::Constant>(3.5, FullID{3, 5}, fluir::FlowGraphLocation{}),
          FullID{3, 2},
          fluir::FlowGraphLocation{}),
        fa::createDependency<fa::Constant>(4.4, FullID{3, 6}, fluir::FlowGraphLocation{}),
        SHARED_ID,
        fluir::FlowGraphLocation{}),
      fluir::FlowGraphLocation{});
    decl.statements.push_back(std::move(shared));
  }
  {
    auto binaryDependent = fa::createDependency<fa::BinaryOp>(
      fluir::Operator::PLUS,
      fa::createDependency<fa::Constant>(100.0, FullID{3, 3}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::LocalRead>(SHARED_ID.back(), FullID{3, 1}, fluir::FlowGraphLocation{}),
      FullID{3, 1},
      fluir::FlowGraphLocation{});
    decl.statements.push_back(std::move(binaryDependent));
  }
  {
    auto unaryDependent = fa::createDependency<fa::UnaryOp>(
      fluir::Operator::MINUS,
      fa::createDependency<fa::LocalRead>(SHARED_ID.back(), FullID{3, 7}, fluir::FlowGraphLocation{}),
      FullID{3, 7},
      fluir::FlowGraphLocation{});
    decl.statements.push_back(std::move(unaryDependent));
  }

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0},
                        .chunks = {fc::Chunk{.name = "bar",
                                             .code =
                                               {
                                                 fc::PUSH,
                                                 0x0,
                                                 fc::F64_NEG,
                                                 fc::PUSH,
                                                 0x1,
                                                 fc::F64_DIV,  // No POP, write {3,4}
                                                 fc::PUSH,
                                                 0x2,
                                                 fc::GET_VAL,  // Read {3,4}
                                                 0x0,
                                                 fc::F64_ADD,
                                                 fc::POP,
                                                 fc::GET_VAL,  // Read {3,4}
                                                 0x0,
                                                 fc::F64_NEG,
                                                 fc::POP,
                                                 fc::POP,
                                                 fc::EXIT,
                                               },
                                             .constants = {3.5_f64, 4.4_f64, 100.0_f64}}}};

  auto typeChecked = fluir::typeCheck(ctx_, std::move(input));
  auto actual = fluir::generateCode(ctx_, typeChecked.value());

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

// TODO: Test with multiple locals together

TEST_F(TestBytecodeGenerator, GeneratesExpressionWithMultipleSharedNodes) {
  const FullID SHARED_1{1, 1};
  const FullID SHARED_2{1, 6};
  fa::AST input;
  input.declarations.push_back([&]() { return fa::Declaration{.id = 1, .name = "main", .statements = {}}; }());
  auto& decl = input.declarations.back();

  {
    auto shared1 = fa::createDependency<fa::LocalWrite>(
      fa::createDependency<fa::UnaryOp>(
        fluir::Operator::MINUS,
        fa::createDependency<fa::BinaryOp>(
          fluir::Operator::STAR,
          fa::createDependency<fa::Constant>(2, FullID{1, 4}, fluir::FlowGraphLocation{}),
          fa::createDependency<fa::Constant>(3, FullID{1, 5}, fluir::FlowGraphLocation{}),
          FullID{1, 3},
          fluir::FlowGraphLocation{}),
        SHARED_1,
        fluir::FlowGraphLocation{}),
      fluir::FlowGraphLocation{});
    decl.statements.push_back(std::move(shared1));
  }
  {
    auto shared2 = fa::createDependency<fa::LocalWrite>(
      fa::createDependency<fa::BinaryOp>(
        fluir::Operator::STAR,
        fa::createDependency<fa::LocalRead>(SHARED_1.back(), FullID{SHARED_2}, fluir::FlowGraphLocation{}),
        fa::createDependency<fa::Constant>(2, FullID{1, 8}, fluir::FlowGraphLocation{}),
        SHARED_2,
        fluir::FlowGraphLocation{}),
      fluir::FlowGraphLocation{});
    decl.statements.push_back(std::move(shared2));
  }
  {
    auto binary = fa::createDependency<fa::BinaryOp>(
      fluir::Operator::PLUS,
      fa::createDependency<fa::LocalRead>(SHARED_2.back(), FullID{1, 8}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::LocalRead>(SHARED_1.back(), FullID{1, 8}, fluir::FlowGraphLocation{}),
      FullID{1, 8},
      fluir::FlowGraphLocation{});
    decl.statements.push_back(std::move(binary));
  }

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0},
                        .chunks = {fc::Chunk{
                          .name = "main",
                          .code =
                            {
                              fc::PUSH, 0x0,         fc::PUSH, 0x1,         fc::I64_MUL, fc::I64_NEG, fc::GET_VAL,
                              0x0,      fc::PUSH,    0x0,      fc::I64_MUL, fc::GET_VAL, 0x1,         fc::GET_VAL,
                              0x0,      fc::I64_ADD, fc::POP,  fc::POP,     fc::POP,     fc::EXIT,
                            },
                          .constants = {2_i32, 3_i32},
                        }}};

  auto typeChecked = fluir::typeCheck(ctx_, std::move(input));
  auto actual = fluir::generateCode(ctx_, typeChecked.value());

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST_F(TestBytecodeGenerator, GeneratesIntConstants) {
  fa::AST input;
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "ints", .statements = {}};
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<I8>(8), FullID{3, 1}, fluir::FlowGraphLocation{})));
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<I16>(16), FullID{3, 2}, fluir::FlowGraphLocation{})));
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<I32>(32), FullID{3, 3}, fluir::FlowGraphLocation{})));
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<I64>(64), FullID{3, 4}, fluir::FlowGraphLocation{})));
    return decl;
  }());

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0},
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

  auto actual = fluir::generateCode(ctx_, input);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST_F(TestBytecodeGenerator, GeneratesUintConstants) {
  fa::AST input;
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "ints", .statements = {}};
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<U8>(8), FullID{3, 1}, fluir::FlowGraphLocation{})));
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<U16>(16), FullID{3, 2}, fluir::FlowGraphLocation{})));
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<U32>(32), FullID{3, 3}, fluir::FlowGraphLocation{})));
    decl.statements.push_back(
      std::move(std::make_unique<fa::Constant>(static_cast<U64>(64), FullID{3, 4}, fluir::FlowGraphLocation{})));

    return decl;
  }());

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0},
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

  auto actual = fluir::generateCode(ctx_, input);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST_F(TestBytecodeGenerator, GeneratesIntBinaryExpression) {
  fa::AST input;
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "ints", .statements = {}};
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::PLUS,
      fa::createDependency<fa::Constant>(static_cast<I32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<I32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::MINUS,
      fa::createDependency<fa::Constant>(static_cast<I32>(28), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<I32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::STAR,
      fa::createDependency<fa::Constant>(static_cast<I32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<I32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::SLASH,
      fa::createDependency<fa::Constant>(static_cast<I32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<I32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));

    return decl;
  }());

  fc::ByteCode expected{
    .header = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0},
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

  auto actual = fluir::generateCode(ctx_, input);
  EXPECT_FALSE(sink_.containsErrors());

  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST_F(TestBytecodeGenerator, GeneratesUintBinaryExpression) {
  fa::AST input;
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "ints", .statements = {}};
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::PLUS,
      fa::createDependency<fa::Constant>(static_cast<U32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<U32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::MINUS,
      fa::createDependency<fa::Constant>(static_cast<U32>(28), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<U32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::STAR,
      fa::createDependency<fa::Constant>(static_cast<U32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<U32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::SLASH,
      fa::createDependency<fa::Constant>(static_cast<U32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<U32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));

    return decl;
  }());

  fc::ByteCode expected{
    .header = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0},
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

  auto actual = fluir::generateCode(ctx_, input);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST_F(TestBytecodeGenerator, GeneratesIntCasts) {
  fa::AST input;
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "ints", .statements = {}};

    auto integer = fa::createDependency<fa::Constant>(static_cast<I32>(8), FullID{3, 1}, fluir::FlowGraphLocation{});
    auto floatPoint = fa::createDependency<fa::Constant>(12.4, FullID{3, 3}, fluir::FlowGraphLocation{});
    decl.statements.push_back(
      std::make_unique<fa::Cast>(ft::ID_F64, fa::clone(integer), FullID{3, 2}, fluir::FlowGraphLocation{}));
    decl.statements.push_back(
      std::make_unique<fa::Cast>(ft::ID_I64, fa::clone(floatPoint), FullID{3, 3}, fluir::FlowGraphLocation{}));
    decl.statements.push_back(
      std::make_unique<fa::Cast>(ft::ID_U64, fa::clone(integer), FullID{3, 2}, fluir::FlowGraphLocation{}));
    decl.statements.push_back(
      std::make_unique<fa::Cast>(ft::ID_I8, fa::clone(integer), FullID{3, 2}, fluir::FlowGraphLocation{}));
    decl.statements.push_back(
      std::make_unique<fa::Cast>(ft::ID_I16, fa::clone(integer), FullID{3, 2}, fluir::FlowGraphLocation{}));
    decl.statements.push_back(
      std::make_unique<fa::Cast>(ft::ID_I64, fa::clone(integer), FullID{3, 2}, fluir::FlowGraphLocation{}));

    return decl;
  }());

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0},
                        .chunks = {fc::Chunk{.name = "ints",
                                             .code =
                                               {
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
                                                 fc::Instruction::EXIT,
                                               },
                                             .constants = {8_i32, 12.4_f64}}}};

  auto actual = fluir::generateCode(ctx_, input);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST_F(TestBytecodeGenerator, GeneratesIntToUintCastsWithWidthCasts) {
  fa::AST input;
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "ints", .statements = {}};

    auto integer = fa::createDependency<fa::Constant>(static_cast<I32>(8), FullID{3, 1}, fluir::FlowGraphLocation{});
    decl.statements.push_back(
      std::make_unique<fa::Cast>(ft::ID_U32, fa::clone(integer), FullID{3, 2}, fluir::FlowGraphLocation{}));
    decl.statements.push_back(
      std::make_unique<fa::Cast>(ft::ID_U16, fa::clone(integer), FullID{3, 2}, fluir::FlowGraphLocation{}));
    decl.statements.push_back(
      std::make_unique<fa::Cast>(ft::ID_U8, fa::clone(integer), FullID{3, 2}, fluir::FlowGraphLocation{}));

    return decl;
  }());

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0},
                        .chunks = {fc::Chunk{.name = "ints",
                                             .code =
                                               {
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
                                                 fc::Instruction::EXIT,
                                               },
                                             .constants = {8_i32}}}};

  auto actual = fluir::generateCode(ctx_, input);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST_F(TestBytecodeGenerator, GeneratesUintCasts) {
  fa::AST input;
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "uints", .statements = {}};

    auto integer = fa::createDependency<fa::Constant>(static_cast<U32>(8), FullID{3, 1}, fluir::FlowGraphLocation{});
    auto floatPoint = fa::createDependency<fa::Constant>(12.4, FullID{3, 3}, fluir::FlowGraphLocation{});
    decl.statements.push_back(
      std::make_unique<fa::Cast>(ft::ID_F64, std::move(integer), FullID{3, 2}, fluir::FlowGraphLocation{}));
    decl.statements.push_back(
      std::make_unique<fa::Cast>(ft::ID_U64, std::move(floatPoint), FullID{3, 3}, fluir::FlowGraphLocation{}));

    return decl;
  }());

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0},
                        .chunks = {fc::Chunk{.name = "uints",
                                             .code =
                                               {
                                                 fc::Instruction::PUSH,
                                                 0x00,
                                                 fc::Instruction::CAST_UF,
                                                 fc::Instruction::POP,
                                                 fc::Instruction::PUSH,
                                                 0x01,
                                                 fc::Instruction::CAST_FU,
                                                 fc::NumericWidth::WIDTH_64,
                                                 fc::Instruction::POP,
                                                 fc::Instruction::EXIT,
                                               },
                                             .constants = {8_u32, 12.4_f64}}}};

  auto actual = fluir::generateCode(ctx_, input);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST_F(TestBytecodeGenerator, GeneratesUintToIntCastsWithWidthCasts) {
  fa::AST input;
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "uints", .statements = {}};

    auto integer = fa::createDependency<fa::Constant>(static_cast<U32>(8), FullID{3, 1}, fluir::FlowGraphLocation{});
    decl.statements.push_back(
      std::make_unique<fa::Cast>(ft::ID_I32, fa::clone(integer), FullID{3, 2}, fluir::FlowGraphLocation{}));
    decl.statements.push_back(
      std::make_unique<fa::Cast>(ft::ID_I16, fa::clone(integer), FullID{3, 2}, fluir::FlowGraphLocation{}));
    decl.statements.push_back(
      std::make_unique<fa::Cast>(ft::ID_I8, fa::clone(integer), FullID{3, 2}, fluir::FlowGraphLocation{}));

    return decl;
  }());

  fc::ByteCode expected{.header = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0},
                        .chunks = {fc::Chunk{.name = "uints",
                                             .code =
                                               {
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
                                                 fc::Instruction::EXIT,
                                               },
                                             .constants = {8_u32}}}};

  auto actual = fluir::generateCode(ctx_, input);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}

TEST_F(TestBytecodeGenerator, GeneratesIncrementDecrementOperations) {
  fa::AST input;
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "inc_dec", .statements = {}};

    auto integer = fa::createDependency<fa::Constant>(static_cast<I32>(8), FullID{3, 1}, fluir::FlowGraphLocation{});
    auto floatingPt = fa::createDependency<fa::Constant>(12.45, FullID{3, 2}, fluir::FlowGraphLocation{});
    auto unsignedInt =
      fa::createDependency<fa::Constant>(static_cast<U64>(8), FullID{3, 3}, fluir::FlowGraphLocation{});

    decl.statements.push_back(std::make_unique<fa::UnaryOp>(
      fluir::Operator::PLUS_PLUS, fa::clone(integer), FullID{3, 4}, fluir::FlowGraphLocation{}));
    decl.statements.push_back(std::make_unique<fa::UnaryOp>(
      fluir::Operator::MINUS_MINUS, fa::clone(integer), FullID{3, 5}, fluir::FlowGraphLocation{}));
    decl.statements.push_back(std::make_unique<fa::UnaryOp>(
      fluir::Operator::PLUS_PLUS, fa::clone(floatingPt), FullID{3, 6}, fluir::FlowGraphLocation{}));
    decl.statements.push_back(std::make_unique<fa::UnaryOp>(
      fluir::Operator::MINUS_MINUS, fa::clone(floatingPt), FullID{3, 7}, fluir::FlowGraphLocation{}));
    decl.statements.push_back(std::make_unique<fa::UnaryOp>(
      fluir::Operator::PLUS_PLUS, fa::clone(unsignedInt), FullID{3, 8}, fluir::FlowGraphLocation{}));
    decl.statements.push_back(std::make_unique<fa::UnaryOp>(
      fluir::Operator::MINUS_MINUS, fa::clone(unsignedInt), FullID{3, 9}, fluir::FlowGraphLocation{}));

    return decl;
  }());

  fc::ByteCode expected{
    .header = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0},
    .chunks = {fc::Chunk{
      .name = "inc_dec",
      .code =
        {
          fc::PUSH,    0x0,         fc::I64_INC, fc::POP,  fc::PUSH,    0x0,         fc::I64_DEC, fc::POP,  fc::PUSH,
          0x1,         fc::F64_INC, fc::POP,     fc::PUSH, 0x1,         fc::F64_DEC, fc::POP,     fc::PUSH, 0x2,
          fc::U64_INC, fc::POP,     fc::PUSH,    0x2,      fc::U64_DEC, fc::POP,     fc::EXIT,
        },
      .constants = {8_i32, 12.45_f64, 8_u64}}}};

  auto actual = fluir::generateCode(ctx_, input);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expected.header, actual.value().header);
  EXPECT_EQ(expected.chunks.size(), actual.value().chunks.size());
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.value().chunks.at(0));
}
