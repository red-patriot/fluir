#include "lsp/database/pt_analyzer.hpp"

#include <gtest/gtest.h>

#include "file_utility.hpp"
#include "fluir/testing/test_files_dir.hpp"
#include "json_utils.hpp"

namespace fs = std::filesystem;

class TestPtAnalyzer : public ::testing::TestWithParam<fs::path> {
 public:
  // Test files are unversioned, so just ignore version errors...
  fluir::CompilerOptions testOptions_{.developerOptions = {.suppressVersionErrors = true}};

  static fs::path diagnosticsPath(fs::path relative) {
    relative.replace_extension(".diags.json");
    return relative;
  }

  static fs::path symbolsPath(fs::path relative) {
    relative.replace_extension(".symbols.json");
    return relative;
  }
};

TEST_P(TestPtAnalyzer, Test) {
  const auto& testFile = GetParam();
  const auto relativePath = fluir::test::getRelativePath(testFile);
  auto relativeDiagsPath = diagnosticsPath(relativePath);
  auto relativeSymbolsPath = symbolsPath(relativePath);

  auto expectedDiagnostics = fluir::lsp::test::readTestJson(relativeDiagsPath);
  auto expectedSymbols = fluir::lsp::test::readTestJson(relativeSymbolsPath);

  auto contents = fluir::test::readContents(testFile);

  fluir::lsp::PtAnalyzer uut{testOptions_};

  ASSERT_NO_THROW(uut.setFileContents(testFile, std::move(contents)));

  const auto diagnostics = uut.diagnostics(testFile);
  const auto symbols = uut.allSymbols(testFile);

  auto actualDiagnostics = fluir::lsp::test::toJsonArray(diagnostics);
  EXPECT_EQ(expectedDiagnostics, actualDiagnostics);

  ASSERT_TRUE(symbols.has_value());
  auto actualSymbols = fluir::lsp::test::toJsonArray(*symbols);
  EXPECT_EQ(expectedSymbols, actualSymbols);
}

INSTANTIATE_TEST_SUITE_P(FullyTypeChecked,
                         TestPtAnalyzer,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("type_check")),
                         fluir::test::filePathName);

INSTANTIATE_TEST_SUITE_P(PartiallyTypeChecked,
                         TestPtAnalyzer,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("type_errors")),
                         fluir::test::filePathName);
