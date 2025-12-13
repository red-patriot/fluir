#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "compiler/frontend/asg_builder.hpp"
#include "compiler/frontend/parser.hpp"
#include "file_utility.hpp"
#include "test_diagnostic_sink.hpp"

namespace fd = fluir::diagnostic;
namespace fs = std::filesystem;

class TestASGError : public ::testing::TestWithParam<fs::path> {
 public:
  fluir::test::TestDiagnosticSink sink_;
  fluir::Context ctx_{.diag = sink_};

  std::optional<fluir::asg::AbstractSyntaxGraph> buildGraph(const fs::path& programFile) {
    if (auto parsed = fluir::parseFile(ctx_, programFile); parsed) {
      return fluir::buildGraph(ctx_, parsed.value());
    }

    return std::nullopt;
  }
};

TEST_P(TestASGError, Test) {
  const fs::path programFile = GetParam();
  const auto errorsFile = fs::path{programFile}.replace_extension(".errors");
  const auto errors = fluir::test::getErrors(errorsFile);

  auto results = buildGraph(programFile);

  std::vector<fd::Code> actual(sink_.emitted().size());
  std::ranges::transform(sink_.emitted(), actual.begin(), [&](const auto& e) { return e.code; });

  EXPECT_FALSE(results.has_value());
  EXPECT_EQ(errors, actual);
}

INSTANTIATE_TEST_SUITE_P(TestASGError,
                         TestASGError,
                         ::testing::ValuesIn(fluir::test::getTestPrograms("build_asg_errors")),
                         fluir::test::filePathName);
