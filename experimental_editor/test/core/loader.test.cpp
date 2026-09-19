#include "editor/core/loader.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/utility/context.hpp"
#include "editor/core/collecting_sink.hpp"

namespace {

  namespace fs = std::filesystem;

  // Fixtures the current compiler frontend cannot parse yet. P1a pins them as
  // expected failures; when the relevant parser support lands, the matching
  // entry must be deliberately removed so the fixture moves into the load set.
  //
  //   branching_conduits, conduits            -- use <segment>, which the parser rejects
  //                                              with ERROR_UNEXPECTED_ELEMENT.
  //   function_with_input_and_output,         -- declare two <return> elements in one
  //   inputs_and_outputs                         <output>; the parser allows a single
  //                                              return only (ERROR_TOO_MANY_RETURNS).
  const std::vector<std::string> kExpectedFailures{
    "branching_conduits", "conduits", "function_with_input_and_output", "inputs_and_outputs"};

  std::string readFile(const fs::path& path) {
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
  }

  std::vector<fs::path> collectFixtures() {
    std::vector<fs::path> fixtures;
    for (const char* sub : {"read", "write"}) {
      const fs::path dir = fs::path(TEST_FOLDER) / sub;
      for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".fl") {
          fixtures.push_back(entry.path());
        }
      }
    }
    std::sort(fixtures.begin(), fixtures.end());
    return fixtures;
  }

  bool isExpectedFailure(const fs::path& path) {
    return std::find(kExpectedFailures.begin(), kExpectedFailures.end(), path.stem().string()) !=
           kExpectedFailures.end();
  }

  std::vector<fs::path> loadSet() {
    std::vector<fs::path> fixtures;
    for (const auto& path : collectFixtures()) {
      if (!isExpectedFailure(path)) {
        fixtures.push_back(path);
      }
    }
    return fixtures;
  }

  std::vector<fs::path> failSet() {
    std::vector<fs::path> fixtures;
    for (const auto& path : collectFixtures()) {
      if (isExpectedFailure(path)) {
        fixtures.push_back(path);
      }
    }
    return fixtures;
  }

  std::string paramName(const testing::TestParamInfo<fs::path>& info) {
    return info.param.parent_path().filename().string() + "_" + info.param.stem().string();
  }

  // Each load needs its own fluir::Context + CollectingSink. The loader does not
  // configure the context, so the caller must: bind the sink and turn version
  // checks off (fixtures declare <version>0.1.3</version>). Every field is named
  // so -Werror=missing-field-initializers stays quiet under the strict warnings.
  struct Loaded {
    fluir::editor::CollectingSink sink;
    fluir::editor::LoadResult result;
  };

  Loaded loadFixtureFile(const fs::path& path) {
    Loaded l;
    fluir::Context ctx{
      .diagnosticSink = l.sink,
      .symbolTable = {},
      .currentFile = {},
      .outputFilename = {},
      .version = {},
      .ignoreVersionChecks = true,
    };
    l.result = fluir::editor::loadFile(ctx, path);
    return l;
  }

  Loaded loadFixtureString(std::string_view source) {
    Loaded l;
    fluir::Context ctx{
      .diagnosticSink = l.sink,
      .symbolTable = {},
      .currentFile = {},
      .outputFilename = {},
      .version = {},
      .ignoreVersionChecks = true,
    };
    l.result = fluir::editor::loadString(ctx, source);
    return l;
  }

}  // namespace

class LoaderLoadSet : public testing::TestWithParam<fs::path> { };

TEST_P(LoaderLoadSet, ParsesSuccessfully) {
  const Loaded l = loadFixtureFile(GetParam());
  EXPECT_TRUE(l.result.tree.has_value()) << "failed to load " << GetParam().string();
}

INSTANTIATE_TEST_SUITE_P(Fixtures, LoaderLoadSet, testing::ValuesIn(loadSet()), paramName);

class LoaderFailSet : public testing::TestWithParam<fs::path> { };

// These fixtures exercise constructs the current parser rejects (see
// kExpectedFailures for the per-fixture reason). If parser support lands for
// one, drop it from kExpectedFailures so it is asserted to load instead.
TEST_P(LoaderFailSet, FailsWithDiagnostics) {
  const Loaded l = loadFixtureFile(GetParam());
  EXPECT_FALSE(l.result.tree.has_value()) << "unexpectedly loaded " << GetParam().string();
  EXPECT_FALSE(l.sink.empty()) << "no diagnostics for " << GetParam().string();
}

INSTANTIATE_TEST_SUITE_P(Fixtures, LoaderFailSet, testing::ValuesIn(failSet()), paramName);

TEST(LoaderString, ParsesSimpleBinaryExpr) {
  const std::string source = readFile(fs::path(TEST_FOLDER) / "read" / "simple_binary_expr.fl");
  ASSERT_FALSE(source.empty());
  const Loaded l = loadFixtureString(source);
  EXPECT_TRUE(l.result.tree.has_value());
}
