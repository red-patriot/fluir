#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "compiler/debug/ast_printer.hpp"
#include "compiler/frontend/ast_builder.hpp"
#include "compiler/frontend/parser.hpp"
#include "file_utility.hpp"
#include "test_diagnostic_sink.hpp"

namespace fs = std::filesystem;

class TestAstParserIntegration : public ::testing::TestWithParam<fs::path> { };

TEST_P(TestAstParserIntegration, Test) {
  const auto& programFile = GetParam();
  const auto outputFile = fluir::test::getGoldenFile(programFile, ".ast");
  const auto expected = fluir::test::readContents(outputFile);

  fluir::test::TestDiagnosticSink sink{};
  fluir::Context ctx{.diagnosticSink = sink};

  auto pt = fluir::parseFile(ctx, programFile);
  ASSERT_TRUE(pt.has_value());
  auto results = fluir::buildGraph(ctx, pt.value());

  ASSERT_TRUE(results.has_value());
  std::stringstream ss;
  fluir::debug::AstPrinter printer{ss, true};
  printer.print(results.value());

  auto actual = ss.str();

  EXPECT_EQ(expected, actual);
}

INSTANTIATE_TEST_SUITE_P(TestAstParserIntegration,
                         TestAstParserIntegration,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("ast")),
                         fluir::test::filePathName);
