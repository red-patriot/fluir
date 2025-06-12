#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "file_utility.hpp"

import fluir.frontend.parser;

namespace fs = std::filesystem;

class TestDetectSyntaxError : public ::testing::TestWithParam<fs::path> { };

TEST_P(TestDetectSyntaxError, Test) {
  const auto programFile = GetParam();
  const auto errorsFile = fs::path{programFile}.replace_extension(".errors");
  const auto errors = fluir::test::readContents(errorsFile);

  fluir::Parser uut;

  auto results = uut.parseFile(programFile);

  std::stringstream ss;
  for (const auto& diagnostic : results.diagnostics()) {
    ss << fluir::toString(diagnostic) << '\n';
  }

  auto actual = ss.str();

  EXPECT_FALSE(results.hasValue());
  EXPECT_EQ(errors, actual);
}

INSTANTIATE_TEST_SUITE_P(TestDetectSyntaxError,
                         TestDetectSyntaxError,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("syntax_error")),
                         fluir::test::filePathName);
