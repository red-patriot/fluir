#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "compiler/frontend/ast_builder.hpp"
#include "compiler/frontend/parser.hpp"
#include "file_utility.hpp"
#include "test_diagnostic_sink.hpp"

namespace fd = fluir::diagnostic;
namespace fs = std::filesystem;

class TestASTError : public ::testing::TestWithParam<fs::path> {
 public:
  fluir::test::TestDiagnosticSink sink_;
  fluir::Context ctx_{.diagnosticSink = sink_};

  std::optional<fluir::ast::AbstractSyntaxTree> buildGraph(const fs::path& programFile) {
    if (auto parsed = fluir::parseFile(ctx_, programFile); parsed) {
      return fluir::buildGraph(ctx_, parsed.value());
    }

    return std::nullopt;
  }
};

TEST_P(TestASTError, Test) {
  const fs::path programFile = GetParam();
  const auto errorsFile = fluir::test::getGoldenFile(programFile, ".errors");
  const auto errors = fluir::test::getErrors(errorsFile);

  auto results = buildGraph(programFile);

  std::vector<fd::Code> actual(sink_.emitted().size());
  std::ranges::transform(sink_.emitted(), actual.begin(), [&](const auto& e) { return e.code; });

  EXPECT_FALSE(results.has_value());
  EXPECT_EQ(errors, actual);
}

INSTANTIATE_TEST_SUITE_P(TestASTError,
                         TestASTError,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("build_ast_errors")),
                         fluir::test::filePathName);
