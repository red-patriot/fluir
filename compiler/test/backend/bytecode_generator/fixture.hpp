#ifndef FLUIR_COMPILER_TEST_BACKEND_BYTECODE_GENERATOR_FIXTURE_HPP
#define FLUIR_COMPILER_TEST_BACKEND_BYTECODE_GENERATOR_FIXTURE_HPP

#include <cassert>
#include <iostream>
#include <vector>

#include <gtest/gtest.h>

#include "bytecode_assertions.hpp"
#include "compiler/backend/bytecode_generator.hpp"
#include "compiler/backend/code_writer.hpp"
#include "compiler/frontend/type_checker.hpp"
#include "compiler/types/builtin_symbols.hpp"
#include "test_diagnostic_sink.hpp"

namespace fluir::test {
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
}  // namespace fluir::test

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

  fluir::test::TestWriter generate(fluir::ast::AST ast) {
    auto checked = prepare(std::move(ast));
    fluir::test::TestWriter writer;
    fluir::generateCode(ctx_, checked, writer);
    EXPECT_FALSE(sink_.containsErrors());
    return writer;
  }

  fluir::code::Header expectedHeader() const {
    return fluir::code::Header{
      .filetype = '\0', .major = ctx_.version.major, .minor = ctx_.version.minor, .patch = ctx_.version.patch};
  }
};

#endif
