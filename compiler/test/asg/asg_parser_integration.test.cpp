#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "compiler/debug/asg_printer.hpp"
#include "file_utility.hpp"

import fluir.frontend;

namespace fs = std::filesystem;

class TestAsgParserIntegration : public ::testing::TestWithParam<fs::path> { };

TEST_P(TestAsgParserIntegration, Test) {
  const auto programFile = GetParam();
  const auto outputFile = fs::path{programFile}.replace_extension(".asg");
  const auto expected = fluir::test::readContents(outputFile);

  auto parseResults = fluir::parseFile(programFile);
  auto results = fluir::buildGraph(parseResults.value());

  std::stringstream ss;
  fluir::debug::AsgPrinter printer{ss, true};
  printer.print(results.value());

  auto actual = ss.str();

  EXPECT_EQ(expected, actual);
}

INSTANTIATE_TEST_SUITE_P(TestAsgParserIntegration,
                         TestAsgParserIntegration,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("asg")),
                         fluir::test::filePathName);
