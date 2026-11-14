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
#include "fixture_loader.hpp"

namespace {

  namespace fs = std::filesystem;

  std::string readFile(const fs::path& path) {
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
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

INSTANTIATE_TEST_SUITE_P(Fixtures,
                         LoaderLoadSet,
                         testing::ValuesIn(testutil::readWriteFixtures(true)),
                         testutil::fixtureName);

class LoaderFailSet : public testing::TestWithParam<fs::path> { };

// These fixtures exercise constructs the current parser rejects (see
// testutil::UNPARSEABLE for the per-fixture reason). If parser support lands for
// one, drop it from UNPARSEABLE so it is asserted to load instead.
TEST_P(LoaderFailSet, FailsWithDiagnostics) {
  const Loaded l = loadFixtureFile(GetParam());
  EXPECT_FALSE(l.result.tree.has_value()) << "unexpectedly loaded " << GetParam().string();
  EXPECT_FALSE(l.sink.empty()) << "no diagnostics for " << GetParam().string();
}

INSTANTIATE_TEST_SUITE_P(Fixtures,
                         LoaderFailSet,
                         testing::ValuesIn(testutil::readWriteFixtures(false)),
                         testutil::fixtureName);

TEST(LoaderString, ParsesSimpleBinaryExpr) {
  const std::string source = readFile(fs::path(TEST_FOLDER) / "read" / "simple_binary_expr.fl");
  ASSERT_FALSE(source.empty());
  const Loaded l = loadFixtureString(source);
  EXPECT_TRUE(l.result.tree.has_value());
}
