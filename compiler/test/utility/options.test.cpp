#include "compiler/utility/options.hpp"

#include <string>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

using fluir::CompilerOptions;
using std::tuple;
using std::vector;
using namespace std::string_literals;

class TestParseOptions : public ::testing::TestWithParam<tuple<fluir::CompilerOptions, std::vector<std::string>>> { };

TEST_P(TestParseOptions, Test) {
  const auto& [expected, args] = GetParam();

  const auto actual = fluir::parseArgs(args);

  EXPECT_EQ(expected, actual);
}

INSTANTIATE_TEST_SUITE_P(
  TestParseOptions,
  TestParseOptions,
  ::testing::Values(tuple{CompilerOptions{.inputFilename = "test_file.fl"s, .outputFilename = "filename.flc"s},
                          vector{"fake_exe_path_for_argc"s, "--output"s, "filename.flc"s, "test_file.fl"s}},
                    tuple{CompilerOptions{.inputFilename = "test_file.fl"s, .outputFilename = "filename.flc"s},
                          vector{"fake_exe_path_for_argc"s, "-o"s, "filename.flc"s, "test_file.fl"s}},
                    tuple{CompilerOptions{.inputFilename = "test_file.fl"s, .colorOutput = false},
                          vector{"fake_exe_path_for_argc"s, "test_file.fl"s, "--no-color"s}}));
