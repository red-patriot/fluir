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

  uut.setFileContents(testFile, std::move(contents));

  EXPECT_NO_THROW(uut.parsed(testFile));
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

  EXPECT_NO_THROW(uut.resolved(testFile));
  EXPECT_NO_THROW(uut.typechecked(testFile));
}

INSTANTIATE_TEST_SUITE_P(TestInMemoryTypeChecking,
                         TestInMemoryTypeChecking,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("type_check")),
                         fluir::test::filePathName);

// using TestInMemoryHandlesDiagnostics = TestInMemoryDb;
//
// TEST_P(TestInMemoryHandlesDiagnostics, Test) {
//   const auto& testFile = GetParam();
//   auto contents = fluir::test::readContents(testFile);
//
//   fluir::lsp::InMemoryDB uut{testOptions_};
//
//   uut.setFileContents(testFile, std::move(contents));
//
//   const auto& diagnostics = uut.diagnostics(testFile);
//
//   EXPECT_FALSE(diagnostics.empty());
// }
//
// INSTANTIATE_TEST_SUITE_P(TestInMemoryHandlesDiagnostics,
//                          TestInMemoryHandlesDiagnostics,
//                          ::testing::ValuesIn(fluir::test::getTestPrograms("syntax_error")),
//                          fluir::test::filePathName);
