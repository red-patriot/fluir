#include "compiler/frontend/parser.hpp"

#include <string>
#include <tuple>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/debug/parse_tree_printer.hpp"
#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "file_utility.hpp"

using std::tuple;
namespace fs = std::filesystem;

class TestParser : public ::testing::TestWithParam<fs::path> { };

TEST_P(TestParser, Test) {
  const fs::path programFile = GetParam();
  const auto outputFile = fs::path{programFile}.replace_extension(".pt");
  const auto expected = fluir::test::readContents(outputFile);

  fluir::Context ctx{.version = fluir::Version{0, 1, 3}};

  auto results = fluir::parseFile(ctx, programFile);

  std::stringstream ss;
  fluir::debug::ParseTreePrinter printer{ss};
  ASSERT_TRUE(results.has_value());
  printer.print(results.value());

  auto actual = ss.str();

  EXPECT_EQ(expected, actual);
}

INSTANTIATE_TEST_SUITE_P(TestParser,
                         TestParser,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("parse")),
                         fluir::test::filePathName);
