#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "compiler/frontend/ast_builder.hpp"
#include "compiler/frontend/parser.hpp"
#include "compiler/frontend/type_checker.hpp"
#include "compiler/types/builtin_symbols.hpp"
#include "file_utility.hpp"
#include "test_diagnostic_sink.hpp"

namespace fs = std::filesystem;
namespace fd = fluir::diagnostic;

class TestTypeError : public ::testing::TestWithParam<fs::path> {
 public:
  fluir::test::TestDiagnosticSink sink_;
  fluir::Context ctx_{.diagnosticSink = sink_, .symbolTable = fluir::types::buildSymbolTable()};

  fluir::Results<fluir::ast::AST> generateAndTypeCheck(const fs::path& programFile) {
    if (auto pt = fluir::parseFile(ctx_, programFile); pt) {
      if (auto ast = fluir::buildGraph(ctx_, *pt); ast) {
        return fluir::typeCheck(ctx_, std::move(*ast));
      }
    }
    return fluir::NoResult;
  }
};

TEST_P(TestTypeError, Test) {
  const fs::path programFile = GetParam();
  const auto errorsFile = fluir::test::getGoldenFile(programFile, ".errors");
  const auto errors = fluir::test::getErrors(errorsFile);

  auto results = generateAndTypeCheck(programFile);
  std::vector<fd::Code> actual(sink_.emitted().size());
  std::ranges::transform(sink_.emitted(), actual.begin(), [](const auto& e) { return e.code; });

  EXPECT_FALSE(results.has_value());
  EXPECT_EQ(errors, actual);
}

INSTANTIATE_TEST_SUITE_P(TestASTError,
                         TestTypeError,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("type_errors")),
                         fluir::test::filePathName);
