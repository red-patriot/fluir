#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "file_utility.hpp"

import fluir.frontend;

namespace fs = std::filesystem;

class TestASGError : public ::testing::TestWithParam<fs::path> { };

TEST_P(TestASGError, Test) {
  const auto programFile = GetParam();
  const auto errorsFile = fs::path{programFile}.replace_extension(".errors");
  const auto errors = fluir::test::readContents(errorsFile);

  auto parseResults = fluir::parseFile(programFile);
  auto results = fluir::buildGraph(parseResults.value());

  std::stringstream ss;
  for (const auto& diagnostic : results.diagnostics()) {
    ss << fluir::toString(diagnostic) << '\n';
  }

  auto actual = ss.str();

  EXPECT_FALSE(results.hasValue());
  EXPECT_EQ(errors, actual);
}

INSTANTIATE_TEST_SUITE_P(TestASGError,
                         TestASGError,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("build_asg_errors")),
                         fluir::test::filePathName);
