#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

#include <gtest/gtest.h>

#include "compiler/frontend/parser.hpp"
#include "compiler/utility/context.hpp"
#include "file_utility.hpp"
#include "test_diagnostic_sink.hpp"

namespace fd = fluir::diagnostic;
namespace fs = std::filesystem;

class TestDetectSyntaxError : public ::testing::TestWithParam<fs::path> {
 public:
  fluir::test::TestDiagnosticSink sink_;
  fluir::Context ctx{.diagnosticSink = sink_, .version = fluir::Version{0, 1, 3}};
};

TEST_P(TestDetectSyntaxError, Test) {
  const auto programFile = GetParam();
  const auto errorsFile = fs::path{programFile}.replace_extension(".errors");
  const auto errors = fluir::test::getLineErrors(errorsFile);

  fluir::Parser uut{ctx};

  auto results = uut.parseFile(programFile);

  std::unordered_multimap<fd::Code, int> actual;
  std::ranges::transform(
    sink_.emitted(), std::inserter(actual, actual.begin()), [&](const auto& data) -> std::pair<const fd::Code, int> {
      const auto& [code, location] = data;
      if (const auto* line = std::get_if<int>(&location); line) {
        return {code, *line};
      }
      ADD_FAILURE();
      return {code, {}};
    });

  EXPECT_FALSE(results.has_value());
  EXPECT_EQ(errors, actual);
}

INSTANTIATE_TEST_SUITE_P(TestDetectSyntaxError,
                         TestDetectSyntaxError,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("syntax_error")),
                         fluir::test::filePathName);
