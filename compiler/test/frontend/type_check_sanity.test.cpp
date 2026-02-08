#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "compiler/debug/ast_printer.hpp"
#include "compiler/frontend/ast_builder.hpp"
#include "compiler/frontend/parser.hpp"
#include "compiler/frontend/type_checker.hpp"
#include "compiler/types/builtin_symbols.hpp"
#include "file_utility.hpp"
#include "test_diagnostic_sink.hpp"

namespace fs = std::filesystem;

/** A test suite that the given files should pass type checking */
class TypeCheckerSanityCheck : public ::testing::TestWithParam<fs::path> { };
// TODO: This could be replaced with an e2e compiler test

TEST_P(TypeCheckerSanityCheck, Test) {
  const auto& programFile = GetParam();

  fluir::test::TestDiagnosticSink sink{};
  fluir::Context ctx{.diagnosticSink = sink, .symbolTable = fluir::types::buildSymbolTable()};

  auto pt = fluir::parseFile(ctx, programFile);
  ASSERT_TRUE(pt.has_value());
  auto ast = fluir::buildGraph(ctx, pt.value());
  ASSERT_TRUE(ast.has_value());
  auto typeChecked = fluir::typeCheck(ctx, std::move(*ast));

  EXPECT_TRUE(typeChecked.has_value());
}

INSTANTIATE_TEST_SUITE_P(TypeCheckerSanityCheck,
                         TypeCheckerSanityCheck,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("type_check")),
                         fluir::test::filePathName);
