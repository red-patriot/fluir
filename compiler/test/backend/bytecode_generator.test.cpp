#include "compiler/backend/bytecode_generator.hpp"

#include <numeric>

#include <gtest/gtest.h>

#include "bytecode/primitives.hpp"
#include "bytecode_assertions.hpp"
#include "compiler/backend/code_writer.hpp"
#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/frontend/type_checker.hpp"
#include "compiler/types/builtin_symbols.hpp"
#include "test_diagnostic_sink.hpp"

namespace fa = fluir::ast;
namespace fc = fluir::code;
namespace ft = fluir::types;
using namespace fluir::literals_types;
using namespace std::string_literals;

using fluir::FullID;

namespace {
  class TestWriter : public fluir::CodeWriter {
   public:
    TestWriter() : CodeWriter(std::cout) { }

    void writeHeader(const fluir::code::Header& in) override { header = in; }
    void writeConstants(const fluir::be::ConstantsArray& in) override { constants = in; }
    void writeChunk(const fluir::code::Chunk& in) override { chunks.push_back(in); }

    fluir::code::Header header;
    fluir::be::ConstantsArray constants;
    std::vector<fluir::code::Chunk> chunks;
  };
}  // namespace

class TestBytecodeGenerator : public ::testing::Test {
 public:
  fluir::test::TestDiagnosticSink sink_;
  fluir::Context ctx_{.diagnosticSink = sink_,
                      .symbolTable = fluir::types::buildSymbolTable(),
                      .version = fluir::Version{.major = 0, .minor = 1, .patch = 3}};

  fluir::ast::AST prepare(fluir::ast::AST ast) {
    auto result = fluir::typeCheck(ctx_, std::move(ast));

    assert(result.has_value() && "Type checking failed");
    return std::move(*result);
  }
};

TEST_F(TestBytecodeGenerator, GeneratesEmptyFunction) {
  fa::AST input;
  input.declarations.emplace_back(fa::FunctionDecl{.id = 3, .name = "main", .statements = {}});

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{.name = "main", .code = {fc::Instruction::RETURN}};

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());

  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesEmptyFunctions) {
  fa::AST input;
  input.declarations.emplace_back(fa::FunctionDecl{.id = 3, .name = "main", .statements = {}});
  input.declarations.emplace_back(fa::FunctionDecl{.id = 2, .name = "foo", .statements = {}});

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  std::vector expectedChunks{fc::Chunk{.name = "main", .code = {fc::Instruction::RETURN}},
                             fc::Chunk{.name = "foo", .code = {fc::Instruction::RETURN}}};

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(expectedChunks.size(), writer.chunks.size());
  for (int i = 0; i != expectedChunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expectedChunks.at(i), writer.chunks.at(i));
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

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{
    .name = "foo",
    .code =
      {
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::F64_MUL,
        fc::Instruction::POP,
        fc::Instruction::RETURN,
      },
  };

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
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

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{.name = "bar",
                          .code = {
                            fc::Instruction::PUSH,
                            0x00,
                            fc::Instruction::F64_NEG,
                            fc::Instruction::POP,
                            fc::Instruction::RETURN,
                          }};

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
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

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{.name = "bar",
                          .code = {
                            fc::PUSH,    0x0,         fc::F64_NEG, fc::PUSH,     0x1,
                            fc::F64_DIV,  // No POP, write {3,4}
                            fc::PUSH,    0x2,
                            fc::GET_VAL,  // Read {3,4}
                            0x0,         fc::F64_ADD, fc::POP,
                            fc::GET_VAL,  // Read {3,4}
                            0x0,         fc::F64_NEG, fc::POP,     fc::MULTIPOP, 0x1, fc::RETURN,
                          }};

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

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

  fc::Header expectedHeader = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{
    .name = "main",
    .code = {
      fc::PUSH,    0x0, fc::PUSH,    0x1, fc::I64_MUL, fc::I64_NEG, fc::GET_VAL,  0x0, fc::PUSH,   0x0, fc::I64_MUL,
      fc::GET_VAL, 0x1, fc::GET_VAL, 0x0, fc::I64_ADD, fc::POP,     fc::MULTIPOP, 0x2, fc::RETURN,
    }};

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
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

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{.name = "ints",
                          .code = {
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

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
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

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{.name = "ints",
                          .code = {
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

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
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

  fc::Header expectedHeader = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{
    .name = "ints",
    .code = {
      fc::Instruction::PUSH,   0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::I64_ADD, fc::Instruction::POP,
      fc::Instruction::PUSH,   0x02, fc::Instruction::PUSH, 0x01, fc::Instruction::I64_SUB, fc::Instruction::POP,
      fc::Instruction::PUSH,   0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::I64_MUL, fc::Instruction::POP,
      fc::Instruction::PUSH,   0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::I64_DIV, fc::Instruction::POP,
      fc::Instruction::RETURN,
    }};

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);
  EXPECT_FALSE(sink_.containsErrors());

  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
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

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{
    .name = "ints",
    .code =
      {
        fc::Instruction::PUSH,   0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::U64_ADD, fc::Instruction::POP,
        fc::Instruction::PUSH,   0x02, fc::Instruction::PUSH, 0x01, fc::Instruction::U64_SUB, fc::Instruction::POP,
        fc::Instruction::PUSH,   0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::U64_MUL, fc::Instruction::POP,
        fc::Instruction::PUSH,   0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::U64_DIV, fc::Instruction::POP,
        fc::Instruction::RETURN,
      },
  };

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
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

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{.name = "ints",
                          .code = {
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

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
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

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{.name = "ints",
                          .code = {
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

  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
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

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{.name = "uints",
                          .code = {
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

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
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

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{.name = "uints",
                          .code = {
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

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
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

  fc::Header expectedHeader = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{
    .name = "inc_dec",
    .code =
      {
        fc::PUSH,    0x0,         fc::I64_INC, fc::POP,  fc::PUSH,    0x0,         fc::I64_DEC, fc::POP,  fc::PUSH,
        0x1,         fc::F64_INC, fc::POP,     fc::PUSH, 0x1,         fc::F64_DEC, fc::POP,     fc::PUSH, 0x2,
        fc::U64_INC, fc::POP,     fc::PUSH,    0x2,      fc::U64_DEC, fc::POP,     fc::RETURN,
      },
  };

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, HandlesMissingLocalVariable) {
  fa::AST input;
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "inc_dec", .statements = {}};

    decl.statements.push_back(std::make_unique<fa::LocalRead>(2, FullID{3, 1}, fluir::FlowGraphLocation{}));

    return decl;
  }());

  TestWriter writer;
  EXPECT_THROW(fluir::generateCode(ctx_, input, writer), fluir::diagnostic::InternalError);
}

TEST_F(TestBytecodeGenerator, GeneratesFunctionCalls) {
  fa::AST input;
  input.declarations.emplace_back([]() {
    auto decl = fa::FunctionDecl{.id = 2, .name = "main", .statements = {}};
    std::vector<fa::UniqueNode> args;
    args.emplace_back(
      fa::createDependency<fa::Constant>(static_cast<I32>(2), FullID{2, 1}, fluir::FlowGraphLocation{}));
    args.emplace_back(
      fa::createDependency<fa::Constant>(static_cast<I32>(3), FullID{2, 2}, fluir::FlowGraphLocation{}));

    decl.statements.emplace_back(
      fa::createDependency<fa::Call>("add_nums"s, std::move(args), FullID{2, 3}, fluir::FlowGraphLocation{}));

    return decl;
  }());
  input.declarations.emplace_back([]() {
    auto decl = fa::FunctionDecl{.id = 3, .name = "add_nums", .statements = {}};
    decl.parameters.push_back(fa::FunctionDecl::Parameter{.id = 3, .name = "a", .typeName = "I32"});
    decl.parameters.push_back(fa::FunctionDecl::Parameter{.id = 2, .name = "b", .typeName = "I32"});
    decl.returnValue = fa::FunctionDecl::Return{.id = 1, .typeName = "I32"};

    decl.statements.push_back(fa::createDependency<fa::LocalWrite>(
      fluir::FullID{3, 1},
      fa::createDependency<fa::BinaryOp>(
        fluir::Operator::PLUS,
        fa::createDependency<fa::LocalRead>(3, fluir::FullID{3, 4}, fluir::FlowGraphLocation{}),
        fa::createDependency<fa::LocalRead>(2, fluir::FullID{3, 4}, fluir::FlowGraphLocation{}),
        FullID{3, 4},
        fluir::FlowGraphLocation{}),
      fluir::FlowGraphLocation{}));

    return decl;
  }());

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  std::vector expectedChunks{fc::Chunk{.name = "main",
                                       .code =
                                         {
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
                                           fc::GET_VAL,
                                           0x1,
                                           fc::GET_VAL,
                                           0x2,
                                           fc::I64_ADD,
                                           fc::SET_VAL,
                                           0x0,
                                           fc::MULTIPOP,  // Pop off the returned temporary
                                           0x01,
                                           fc::MULTIPOP,
                                           0x2,
                                           fc::Instruction::RETURN,
                                         },
                                       .inCount = 2,
                                       .outCount = 1}};

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(expectedChunks.size(), writer.chunks.size());
  for (size_t i = 0; i != expectedChunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expectedChunks.at(i), writer.chunks.at(i));
  }
}

TEST_F(TestBytecodeGenerator, GeneratesFunctionCallWithNoReturn) {
  fa::AST input;
  input.declarations.emplace_back([]() {
    auto decl = fa::FunctionDecl{.id = 1, .name = "main", .statements = {}};
    std::vector<fa::UniqueNode> args;
    args.emplace_back(
      fa::createDependency<fa::Constant>(static_cast<I32>(7), FullID{1, 1}, fluir::FlowGraphLocation{}));
    decl.statements.emplace_back(
      fa::createDependency<fa::Call>("sink"s, std::move(args), FullID{1, 2}, fluir::FlowGraphLocation{}));
    return decl;
  }());
  input.declarations.emplace_back([]() {
    auto decl = fa::FunctionDecl{.id = 2, .name = "sink", .statements = {}};
    decl.parameters.push_back(fa::FunctionDecl::Parameter{.id = 3, .name = "x", .typeName = "I32"});
    return decl;
  }());

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  std::vector expectedChunks{fc::Chunk{.name = "main",
                                       .code =
                                         {
                                           fc::PUSH,
                                           0x00,  // arg 7
                                           fc::CALL,
                                           0x00,
                                           0x00,
                                           0x00,
                                           0x01,
                                           // No return => no POP instruction
                                           fc::Instruction::RETURN,
                                         }},
                             fc::Chunk{.name = "sink",
                                       .code =
                                         {
                                           fc::MULTIPOP,
                                           0x01,
                                           fc::Instruction::RETURN,
                                         },
                                       .inCount = 1,
                                       .outCount = 0}};

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(expectedChunks.size(), writer.chunks.size());
  for (int i = 0; i != static_cast<int>(expectedChunks.size()); ++i) {
    EXPECT_CHUNK_EQ(expectedChunks.at(i), writer.chunks.at(i));
  }
}

TEST_F(TestBytecodeGenerator, GeneratesFunctionCallWithNoArguments) {
  fa::AST input;
  input.declarations.emplace_back([]() {
    auto decl = fa::FunctionDecl{.id = 1, .name = "main", .statements = {}};
    decl.statements.emplace_back(fa::createDependency<fa::Call>(
      "get_val"s, std::vector<fa::UniqueNode>{}, FullID{1, 1}, fluir::FlowGraphLocation{}));
    return decl;
  }());
  input.declarations.emplace_back([]() {
    auto decl = fa::FunctionDecl{.id = 2, .name = "get_val", .statements = {}};
    decl.statements.emplace_back(fa::createDependency<fa::LocalWrite>(
      FullID{2, 1},
      fa::createDependency<fa::Constant>(12, FullID{2, 2}, fluir::FlowGraphLocation{}),
      fluir::FlowGraphLocation{}));
    decl.returnValue = fa::FunctionDecl::Return{.id = 1, .typeName = "I32"};
    return decl;
  }());

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  std::vector expectedChunks{fc::Chunk{.name = "main",
                                       .code =
                                         {
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
                                           fc::Instruction::PUSH,
                                           0x0,
                                           fc::Instruction::SET_VAL,
                                           0x0,
                                           fc::Instruction::MULTIPOP,  // Pop off the returned temporary
                                           0x01,
                                           fc::Instruction::RETURN,
                                         },
                                       .inCount = 0,
                                       .outCount = 1}};

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(expectedChunks.size(), writer.chunks.size());
  for (int i = 0; i != static_cast<int>(expectedChunks.size()); ++i) {
    EXPECT_CHUNK_EQ(expectedChunks.at(i), writer.chunks.at(i));
  }
}

TEST_F(TestBytecodeGenerator, GeneratesMultipleFunctionCalls) {
  fa::AST input;
  input.declarations.emplace_back([]() {
    auto decl = fa::FunctionDecl{.id = 1, .name = "main", .statements = {}};
    decl.statements.emplace_back(fa::createDependency<fa::Call>(
      "first"s, std::vector<fa::UniqueNode>{}, FullID{1, 1}, fluir::FlowGraphLocation{}));
    decl.statements.emplace_back(fa::createDependency<fa::Call>(
      "second"s, std::vector<fa::UniqueNode>{}, FullID{1, 2}, fluir::FlowGraphLocation{}));
    return decl;
  }());
  input.declarations.emplace_back([]() {
    auto decl = fa::FunctionDecl{.id = 2, .name = "first", .statements = {}};
    decl.returnValue = fa::FunctionDecl::Return{.id = 1, .typeName = "I32"};
    return decl;
  }());
  input.declarations.emplace_back([]() {
    auto decl = fa::FunctionDecl{.id = 3, .name = "second", .statements = {}};
    decl.returnValue = fa::FunctionDecl::Return{.id = 1, .typeName = "I32"};
    return decl;
  }());

  fc::Header expectedHeader = {.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  std::vector expectedChunks{
    fc::Chunk{.name = "main",
              .code =
                {
                  fc::RESERVE,
                  0x01,
                  fc::CALL,
                  0x00,
                  0x00,
                  0x00,
                  0x01,
                  fc::Instruction::POP,
                  fc::RESERVE,
                  0x01,
                  fc::CALL,
                  0x00,
                  0x00,
                  0x00,
                  0x02,
                  fc::Instruction::POP,
                  fc::Instruction::RETURN,
                }},
    fc::Chunk{.name = "first", .code = {fc::Instruction::RETURN}, .inCount = 0, .outCount = 1},
    fc::Chunk{.name = "second", .code = {fc::Instruction::RETURN}, .inCount = 0, .outCount = 1}};

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(expectedChunks.size(), writer.chunks.size());
  for (int i = 0; i != static_cast<int>(expectedChunks.size()); ++i) {
    EXPECT_CHUNK_EQ(expectedChunks.at(i), writer.chunks.at(i));
  }
}

TEST_F(TestBytecodeGenerator, GeneratesCalleeChunkForFunctionWithParameters) {
  fa::AST input;
  input.declarations.emplace_back([]() {
    auto decl = fa::FunctionDecl{.id = 1, .name = "add", .statements = {}};
    decl.returnValue = fa::FunctionDecl::Return{.id = 1, .typeName = "I32"};
    decl.parameters.push_back(fa::FunctionDecl::Parameter{.id = 2, .name = "a", .typeName = "I32"});
    decl.parameters.push_back(fa::FunctionDecl::Parameter{.id = 3, .name = "b", .typeName = "I32"});
    return decl;
  }());
  input.declarations.emplace_back([]() {
    auto decl = fa::FunctionDecl{.id = 2, .name = "main", .statements = {}};
    std::vector<fa::UniqueNode> args;
    args.emplace_back(
      fa::createDependency<fa::Constant>(static_cast<I32>(10), FullID{2, 1}, fluir::FlowGraphLocation{}));
    args.emplace_back(
      fa::createDependency<fa::Constant>(static_cast<I32>(20), FullID{2, 2}, fluir::FlowGraphLocation{}));
    decl.statements.emplace_back(
      fa::createDependency<fa::Call>("add"s, std::move(args), FullID{2, 3}, fluir::FlowGraphLocation{}));
    return decl;
  }());

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  std::vector expectedChunks{fc::Chunk{.name = "add",
                                       .code =
                                         {
                                           fc::MULTIPOP,
                                           0x02,
                                           fc::Instruction::RETURN,
                                         },
                                       .inCount = 2,
                                       .outCount = 1},
                             fc::Chunk{.name = "main",
                                       .code = {
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

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(expectedChunks.size(), writer.chunks.size());
  for (int i = 0; i != static_cast<int>(expectedChunks.size()); ++i) {
    EXPECT_CHUNK_EQ(expectedChunks.at(i), writer.chunks.at(i));
  }
}

TEST_F(TestBytecodeGenerator, ConstantsAreGlobal) {
  fa::AST input;
  input.declarations.emplace_back(
    fa::FunctionDecl{.id = 3, .name = "foo", .statements = []() {
                       fa::DataFlowGraph graph;
                       graph.push_back(std::make_unique<fa::BinaryOp>(
                         fluir::Operator::PLUS,
                         fa::createDependency<fa::Constant>(1.0, FullID{3, 2}, fluir::FlowGraphLocation{}),
                         fa::createDependency<fa::Constant>(2.0, FullID{3, 3}, fluir::FlowGraphLocation{}),
                         FullID{3, 1},
                         fluir::FlowGraphLocation{}));
                       return graph;
                     }()});
  input.declarations.emplace_back(
    fa::FunctionDecl{.id = 4, .name = "bar", .statements = []() {
                       fa::DataFlowGraph graph;
                       graph.push_back(std::make_unique<fa::BinaryOp>(
                         fluir::Operator::PLUS,
                         fa::createDependency<fa::Constant>(3.0, FullID{4, 2}, fluir::FlowGraphLocation{}),
                         fa::createDependency<fa::Constant>(4.0, FullID{4, 3}, fluir::FlowGraphLocation{}),
                         FullID{4, 1},
                         fluir::FlowGraphLocation{}));
                       return graph;
                     }()});

  // Global pool: [1.0, 2.0, 3.0, 4.0]
  // foo: PUSH 0, PUSH 1, F64_ADD, POP, RETURN
  // bar: PUSH 2, PUSH 3, F64_ADD, POP, RETURN
  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fluir::be::ConstantsArray expectedConstants{1.0, 2.0, 3.0, 4.0};
  std::vector expectedChunks{
    fc::Chunk{.name = "foo", .code = {fc::PUSH, 0x00, fc::PUSH, 0x01, fc::F64_ADD, fc::POP, fc::RETURN}},
    fc::Chunk{.name = "bar", .code = {fc::PUSH, 0x02, fc::PUSH, 0x03, fc::F64_ADD, fc::POP, fc::RETURN}},
  };

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_VALUES_EQ(expectedConstants, writer.constants);
  for (int i = 0; i != 2; ++i) {
    EXPECT_CHUNK_EQ(expectedChunks.at(i), writer.chunks.at(i));
  }
}

TEST_F(TestBytecodeGenerator, EmitsQPushForLotsOfConstants) {
  size_t constsCount = 257;
  fa::AST input;
  input.declarations.emplace_back(
    fa::FunctionDecl{.id = 3, .name = "foo", .statements = [&]() {
                       fa::DataFlowGraph graph;
                       graph.reserve(constsCount);
                       for (size_t i = 0; i != constsCount; ++i) {
                         graph.emplace_back(std::make_unique<fa::Constant>(i, FullID{i}, fluir::FlowGraphLocation{}));
                       }
                       return graph;
                     }()});

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
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

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_VALUES_EQ(expectedConstants, writer.constants);
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.back());
}

TEST_F(TestBytecodeGenerator, GeneratesCodeForBuiltinFunctions) {
  fa::AST input;
  input.declarations.emplace_back([]() {
    auto decl = fa::FunctionDecl{
      .id = 1,
      .name = "main",
      .statements = {
        []() {
          fluir::ast::DataFlowGraph graph;

          std::vector<fa::UniqueNode> args;
          args.emplace_back(fa::createDependency<fa::Constant>(12.5, FullID{1, 2}, fluir::FlowGraphLocation{}));
          graph.emplace_back(
            fa::createDependency<fa::Call>("print"s, std::move(args), FullID{1, 1}, fluir::FlowGraphLocation{}));
          return graph;
        }(),
      }};
    return decl;
  }());

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fluir::be::ConstantsArray expectedConstants{12.5, "print"s};
  fc::Chunk expectedChunk{.name = "main",
                          .code = {
                            fc::PUSH,
                            0x00,  // arg 12.5
                            fc::DYN_CALL,
                            0x00,
                            0x00,
                            0x00,
                            0x01,
                            fc::Instruction::RETURN,
                          }};

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);

  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  EXPECT_EQ(writer.constants, expectedConstants);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesEqualityComparisons) {
  fa::AST input;
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "eq", .statements = {}};
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::EQUAL_EQUAL,
      fa::createDependency<fa::Constant>(static_cast<I32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<I32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::BANG_EQUAL,
      fa::createDependency<fa::Constant>(static_cast<I32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<I32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(
      std::make_unique<fa::BinaryOp>(fluir::Operator::EQUAL_EQUAL,
                                     fa::createDependency<fa::Constant>(1.5, FullID{3, 1}, fluir::FlowGraphLocation{}),
                                     fa::createDependency<fa::Constant>(2.5, FullID{3, 2}, fluir::FlowGraphLocation{}),
                                     FullID{3, 3},
                                     fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::EQUAL_EQUAL,
      fa::createDependency<fa::Constant>(true, FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(false, FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));

    return decl;
  }());

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{
    .name = "eq",
    .code =
      {
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::EQ,
        fc::Instruction::POP,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::EQ,
        fc::Instruction::NOT,
        fc::Instruction::POP,
        fc::Instruction::PUSH,
        0x02,
        fc::Instruction::PUSH,
        0x03,
        fc::Instruction::EQ,
        fc::Instruction::POP,
        fc::Instruction::PUSH,
        0x04,
        fc::Instruction::PUSH,
        0x05,
        fc::Instruction::EQ,
        fc::Instruction::POP,
        fc::Instruction::RETURN,
      },
  };

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);
  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesSignedComparisons) {
  fa::AST input;
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "cmp", .statements = {}};
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::LESS,
      fa::createDependency<fa::Constant>(static_cast<I32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<I32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::LESS_EQUAL,
      fa::createDependency<fa::Constant>(static_cast<I32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<I32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    // GREATER swaps operands: rhs (16 = constant index 1) is pushed first.
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::GREATER,
      fa::createDependency<fa::Constant>(static_cast<I32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<I32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    // GREATER_EQUAL swaps operands: rhs (16 = constant index 1) is pushed first.
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::GREATER_EQUAL,
      fa::createDependency<fa::Constant>(static_cast<I32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<I32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));

    return decl;
  }());

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{
    .name = "cmp",
    .code =
      {
        fc::Instruction::PUSH,   0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::I64_LT, fc::Instruction::POP,
        fc::Instruction::PUSH,   0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::I64_LE, fc::Instruction::POP,
        fc::Instruction::PUSH,   0x01, fc::Instruction::PUSH, 0x00, fc::Instruction::I64_LT, fc::Instruction::POP,
        fc::Instruction::PUSH,   0x01, fc::Instruction::PUSH, 0x00, fc::Instruction::I64_LE, fc::Instruction::POP,
        fc::Instruction::RETURN,
      },
  };

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);
  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesUnsignedComparisons) {
  fa::AST input;
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "cmp", .statements = {}};
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::LESS,
      fa::createDependency<fa::Constant>(static_cast<U32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<U32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::LESS_EQUAL,
      fa::createDependency<fa::Constant>(static_cast<U32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<U32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    // GREATER swaps operands: rhs (16 = constant index 1) is pushed first.
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::GREATER,
      fa::createDependency<fa::Constant>(static_cast<U32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<U32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    // GREATER_EQUAL swaps operands: rhs (16 = constant index 1) is pushed first.
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::GREATER_EQUAL,
      fa::createDependency<fa::Constant>(static_cast<U32>(8), FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(static_cast<U32>(16), FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));

    return decl;
  }());

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{
    .name = "cmp",
    .code =
      {
        fc::Instruction::PUSH,   0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::U64_LT, fc::Instruction::POP,
        fc::Instruction::PUSH,   0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::U64_LE, fc::Instruction::POP,
        fc::Instruction::PUSH,   0x01, fc::Instruction::PUSH, 0x00, fc::Instruction::U64_LT, fc::Instruction::POP,
        fc::Instruction::PUSH,   0x01, fc::Instruction::PUSH, 0x00, fc::Instruction::U64_LE, fc::Instruction::POP,
        fc::Instruction::RETURN,
      },
  };

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);
  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesFloatComparisons) {
  fa::AST input;
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "cmp", .statements = {}};
    decl.statements.push_back(std::move(
      std::make_unique<fa::BinaryOp>(fluir::Operator::LESS,
                                     fa::createDependency<fa::Constant>(1.5, FullID{3, 1}, fluir::FlowGraphLocation{}),
                                     fa::createDependency<fa::Constant>(2.5, FullID{3, 2}, fluir::FlowGraphLocation{}),
                                     FullID{3, 3},
                                     fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(
      std::make_unique<fa::BinaryOp>(fluir::Operator::LESS_EQUAL,
                                     fa::createDependency<fa::Constant>(1.5, FullID{3, 1}, fluir::FlowGraphLocation{}),
                                     fa::createDependency<fa::Constant>(2.5, FullID{3, 2}, fluir::FlowGraphLocation{}),
                                     FullID{3, 3},
                                     fluir::FlowGraphLocation{})));
    // GREATER swaps operands: rhs (2.5 = constant index 1) is pushed first.
    decl.statements.push_back(std::move(
      std::make_unique<fa::BinaryOp>(fluir::Operator::GREATER,
                                     fa::createDependency<fa::Constant>(1.5, FullID{3, 1}, fluir::FlowGraphLocation{}),
                                     fa::createDependency<fa::Constant>(2.5, FullID{3, 2}, fluir::FlowGraphLocation{}),
                                     FullID{3, 3},
                                     fluir::FlowGraphLocation{})));
    // GREATER_EQUAL swaps operands: rhs (2.5 = constant index 1) is pushed first.
    decl.statements.push_back(std::move(
      std::make_unique<fa::BinaryOp>(fluir::Operator::GREATER_EQUAL,
                                     fa::createDependency<fa::Constant>(1.5, FullID{3, 1}, fluir::FlowGraphLocation{}),
                                     fa::createDependency<fa::Constant>(2.5, FullID{3, 2}, fluir::FlowGraphLocation{}),
                                     FullID{3, 3},
                                     fluir::FlowGraphLocation{})));

    return decl;
  }());

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{
    .name = "cmp",
    .code =
      {
        fc::Instruction::PUSH,   0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::F64_LT, fc::Instruction::POP,
        fc::Instruction::PUSH,   0x00, fc::Instruction::PUSH, 0x01, fc::Instruction::F64_LE, fc::Instruction::POP,
        fc::Instruction::PUSH,   0x01, fc::Instruction::PUSH, 0x00, fc::Instruction::F64_LT, fc::Instruction::POP,
        fc::Instruction::PUSH,   0x01, fc::Instruction::PUSH, 0x00, fc::Instruction::F64_LE, fc::Instruction::POP,
        fc::Instruction::RETURN,
      },
  };

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);
  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}

TEST_F(TestBytecodeGenerator, GeneratesLogicalOperators) {
  fa::AST input;
  input.declarations.emplace_back([&]() {
    fa::FunctionDecl decl{.id = 3, .name = "logic", .statements = {}};
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::AND_AND,
      fa::createDependency<fa::Constant>(true, FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(false, FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    decl.statements.push_back(std::move(std::make_unique<fa::BinaryOp>(
      fluir::Operator::BAR_BAR,
      fa::createDependency<fa::Constant>(true, FullID{3, 1}, fluir::FlowGraphLocation{}),
      fa::createDependency<fa::Constant>(false, FullID{3, 2}, fluir::FlowGraphLocation{}),
      FullID{3, 3},
      fluir::FlowGraphLocation{})));
    // BANG is unary; this also exercises not-yet-implemented bool-constant emission.
    decl.statements.push_back(
      std::make_unique<fa::UnaryOp>(fluir::Operator::BANG,
                                    fa::createDependency<fa::Constant>(true, FullID{3, 1}, fluir::FlowGraphLocation{}),
                                    FullID{3, 2},
                                    fluir::FlowGraphLocation{}));

    return decl;
  }());

  fc::Header expectedHeader{.filetype = '\0', .major = 0, .minor = 1, .patch = 3, .entryOffset = 0};
  fc::Chunk expectedChunk{
    .name = "logic",
    .code =
      {
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::AND,
        fc::Instruction::POP,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::PUSH,
        0x01,
        fc::Instruction::OR,
        fc::Instruction::POP,
        fc::Instruction::PUSH,
        0x00,
        fc::Instruction::NOT,
        fc::Instruction::POP,
        fc::Instruction::RETURN,
      },
  };

  input = prepare(std::move(input));
  TestWriter writer;
  fluir::generateCode(ctx_, input, writer);
  EXPECT_FALSE(sink_.containsErrors());
  EXPECT_BC_HEADER_EQ(expectedHeader, writer.header);
  ASSERT_EQ(1, writer.chunks.size());
  EXPECT_CHUNK_EQ(expectedChunk, writer.chunks.front());
}
