#include "editor/core/parse_tree_writer.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/utility/context.hpp"
#include "editor/core/collecting_sink.hpp"
#include "editor/core/loader.hpp"
#include "editor/transaction/edit_comment.hpp"

namespace {

  namespace fs = std::filesystem;

  std::string readFile(const fs::path& path) {
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
  }

  // Byte-exact against the legacy write goldens. Excluded, with reason:
  //   multiple_functions, conduits, unary_negate -- legacy emits list order;
  //     ParseTreeWriter emits ascending-id order.
  //   conduits            -- also <f64>6.7890</f64> (trailing zero cannot come
  //                          back from a double) and <segment> (parser rejects).
  //   inputs_and_outputs  -- two <return>; pt::FunctionDecl allows one.
  //   nested_comments     -- body lists constant 2 before comment 1; ascending ids.
  const std::vector<std::string> kGoldenSubset{
    "single_function",
    "binary_add",
    "boolean",
    "call_no_return",
    "call_reorder_args",
    "call_with_args",
    "function_inputs",
    "function_output",
    "signed_constants",
    "unsigned_constants",
    "function_comment",
    "simple_comment",
    "top_level_comment",
    "empty_comment",
  };

  fluir::editor::LoadResult parse(fluir::editor::CollectingSink& sink, std::string_view src) {
    fluir::Context ctx{
      .diagnosticSink = sink,
      .symbolTable = {},
      .currentFile = {},
      .outputFilename = {},
      .version = {},
      .ignoreVersionChecks = true,
    };
    return fluir::editor::loadString(ctx, src);
  }

}  // namespace

class ParseTreeWriterGolden : public testing::TestWithParam<std::string> { };

TEST_P(ParseTreeWriterGolden, MatchesLegacyXml) {
  const fs::path path = fs::path(TEST_FOLDER) / "write" / (GetParam() + ".fl");
  const std::string expected = readFile(path);
  ASSERT_FALSE(expected.empty()) << "missing fixture " << path.string();

  fluir::editor::CollectingSink sink;
  const auto loaded = parse(sink, expected);
  ASSERT_TRUE(loaded.tree.has_value()) << "parse failed for " << path.string();
  EXPECT_TRUE(sink.empty());

  std::ostringstream out;
  fluir::editor::ParseTreeWriter(out).write(*loaded.tree);
  EXPECT_EQ(out.str(), expected);
}

INSTANTIATE_TEST_SUITE_P(Fixtures,
                         ParseTreeWriterGolden,
                         testing::ValuesIn(kGoldenSubset),
                         [](const testing::TestParamInfo<std::string>& i) { return i.param; });

TEST(ParseTreeWriter, WritesEmptyFunctionFromHandBuiltTree) {
  fluir::pt::ParseTree tree;
  tree.header.version = {0, 1, 3};
  fluir::pt::FunctionDecl fn;
  fn.id = 1;
  fn.location = {.x = 10, .y = 10, .z = 3, .width = 100, .height = 100};
  fn.name = "foo";
  tree.declarations.emplace(fn.id, fluir::pt::Declaration{fn});

  std::ostringstream out;
  fluir::editor::ParseTreeWriter(out).write(tree);
  EXPECT_EQ(out.str(), readFile(fs::path(TEST_FOLDER) / "write" / "single_function.fl"));
}

TEST(ParseTreeWriter, WritesToFilePath) {
  const std::string expected = readFile(fs::path(TEST_FOLDER) / "write" / "binary_add.fl");
  fluir::editor::CollectingSink sink;
  const auto loaded = parse(sink, expected);
  ASSERT_TRUE(loaded.tree.has_value());

  const fs::path tmp = fs::temp_directory_path() / "fluir_parse_tree_writer_test.fl";
  {
    std::ofstream ofs(tmp);
    fluir::editor::ParseTreeWriter w(ofs);
    w.write(*loaded.tree);
    EXPECT_TRUE(w.good());
  }
  EXPECT_EQ(readFile(tmp), expected);
  fs::remove(tmp);
}

TEST(ParseTreeWriter, WritesEditedCommentsBackReadably) {
  fluir::editor::CollectingSink sink;
  auto loaded = parse(sink, readFile(fs::path(TEST_FOLDER) / "write" / "nested_comments.fl"));
  ASSERT_TRUE(loaded.tree.has_value());
  fluir::pt::ParseTree edited = *loaded.tree;
  ASSERT_TRUE((fluir::editor::EditCommentTransaction{fluir::FullID{2}, "Top, edited: <&> 'quoted'!"}).execute(edited));
  ASSERT_TRUE(
    (fluir::editor::EditCommentTransaction{fluir::FullID{1, 1}, "  body edit, with spaces  "}).execute(edited));

  std::ostringstream out;
  fluir::editor::ParseTreeWriter(out).write(edited);
  const auto reloaded = parse(sink, out.str());
  ASSERT_TRUE(reloaded.tree.has_value()) << out.str();
  EXPECT_TRUE(sink.empty());
  EXPECT_EQ(*reloaded.tree, edited);
}
