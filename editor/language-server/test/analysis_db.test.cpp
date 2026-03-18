#include <gtest/gtest.h>

#include "fluir/testing/test_files_dir.hpp"
#include "lsp/database/in_memory_db.hpp"

namespace fs = std::filesystem;

class TestInMemoryDb : public ::testing::TestWithParam<fs::path> {
 public:
  // Test files are unversioned, so just ignore version errors...
  fluir::CompilerOptions testOptions_{.developerOptions = {.suppressVersionErrors = true}};
};

using TestInMemoryDbParsing = TestInMemoryDb;
using TestInMemoryTypeChecking = TestInMemoryDb;

TEST_P(TestInMemoryDbParsing, Test) {
  const auto& testFile = GetParam();
  auto contents = fluir::test::readContents(testFile);

  fluir::lsp::InMemoryDB uut{testOptions_};

  EXPECT_NO_THROW(uut.setFileContents(testFile, std::move(contents)));
}

INSTANTIATE_TEST_SUITE_P(TestInMemoryDbParsing,
                         TestInMemoryDbParsing,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("type_check")),
                         fluir::test::filePathName);

TEST_P(TestInMemoryTypeChecking, Test) {
  const auto& testFile = GetParam();
  auto contents = fluir::test::readContents(testFile);

  fluir::lsp::InMemoryDB uut{testOptions_};

  uut.setFileContents(testFile, std::move(contents));

  // Verify the tree was built with declarations and symbols
  const auto& diagnostics = uut.diagnostics(testFile);
  EXPECT_TRUE(diagnostics.empty());
}

INSTANTIATE_TEST_SUITE_P(TestInMemoryTypeChecking,
                         TestInMemoryTypeChecking,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("type_check")),
                         fluir::test::filePathName);

class TestInMemoryHandlesDiagnostics : public ::testing::TestWithParam<fs::path> {
 public:
  // Test files are unversioned, so just ignore version errors...
  fluir::CompilerOptions testOptions_{};
};

TEST_P(TestInMemoryHandlesDiagnostics, Test) {
  const auto& testFile = GetParam();
  auto contents = fluir::test::readContents(testFile);

  fluir::lsp::InMemoryDB uut{testOptions_};

  uut.setFileContents(testFile, std::move(contents));

  const auto& diagnostics = uut.diagnostics(testFile);

  EXPECT_FALSE(diagnostics.empty());
}

INSTANTIATE_TEST_SUITE_P(TestInMemoryHandlesDiagnostics,
                         TestInMemoryHandlesDiagnostics,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("syntax_error")),
                         fluir::test::filePathName);

// --- allSymbols tests ---

TEST(TestAllSymbols, UnknownFileReturnsNullopt) {
  fluir::CompilerOptions opts{.developerOptions = {.suppressVersionErrors = true}};
  fluir::lsp::InMemoryDB uut{opts};

  auto result = uut.allSymbols(fs::path("/nonexistent/file.fl"));
  EXPECT_FALSE(result.has_value());
}

TEST_P(TestInMemoryTypeChecking, AllSymbolsReturnsNonEmptyVector) {
  const auto& testFile = GetParam();
  auto contents = fluir::test::readContents(testFile);

  fluir::lsp::InMemoryDB uut{testOptions_};
  uut.setFileContents(testFile, std::move(contents));

  auto result = uut.allSymbols(testFile);
  ASSERT_TRUE(result.has_value());
  EXPECT_FALSE(result->empty());

  for (const auto& tagged : *result) {
    EXPECT_FALSE(tagged.id.empty());
    EXPECT_FALSE(tagged.symbol.name.empty());
  }
}

using TestAllSymbolsSyntaxError = TestInMemoryHandlesDiagnostics;

TEST_P(TestAllSymbolsSyntaxError, AllSymbolsReturnsValueForKnownFile) {
  const auto& testFile = GetParam();
  auto contents = fluir::test::readContents(testFile);

  fluir::lsp::InMemoryDB uut{testOptions_};
  uut.setFileContents(testFile, std::move(contents));

  auto result = uut.allSymbols(testFile);
  ASSERT_TRUE(result.has_value());
}

INSTANTIATE_TEST_SUITE_P(TestAllSymbolsSyntaxError,
                         TestAllSymbolsSyntaxError,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("syntax_error")),
                         fluir::test::filePathName);
