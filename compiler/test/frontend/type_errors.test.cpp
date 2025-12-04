#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "compiler/frontend/asg_builder.hpp"
#include "compiler/frontend/parser.hpp"
#include "compiler/frontend/type_checker.hpp"
#include "compiler/types/builtin_symbols.hpp"
#include "compiler/utility/pass.hpp"
#include "file_utility.hpp"

namespace fs = std::filesystem;

class TestTypeError : public ::testing::TestWithParam<fs::path> {
 public:
  fluir::Context ctx_{.symbolTable = fluir::types::buildSymbolTable()};
};

TEST_P(TestTypeError, Test) {
  const fs::path programFile = GetParam();
  const auto errorsFile = fs::path{programFile}.replace_extension(".errors");
  const auto errors = fluir::test::readContents(errorsFile);

  auto [ctx, results] =
    fluir::addContext(std::move(ctx_), programFile) | fluir::parseFile | fluir::buildGraph | fluir::typeCheck;

  std::stringstream ss;
  for (const auto& diagnostic : ctx.diagnostics) {
    ss << fluir::toString(diagnostic) << '\n';
  }

  auto actual = ss.str();

  EXPECT_FALSE(results.has_value());
  EXPECT_EQ(errors, actual);
}

INSTANTIATE_TEST_SUITE_P(TestASGError,
                         TestTypeError,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("type_errors")),
                         fluir::test::filePathName);
