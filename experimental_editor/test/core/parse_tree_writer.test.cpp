#include "editor/core/parse_tree_writer.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/utility/context.hpp"
#include "editor/core/collecting_sink.hpp"
#include "editor/core/loader.hpp"
#include "editor/core/tree.hpp"
#include "editor/transaction/edit_comment.hpp"
#include "file_utility.hpp"

namespace {

  namespace fs = std::filesystem;

  std::string readFile(const fs::path& path) {
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
  }

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

class ParseTreeWriterGolden : public testing::TestWithParam<fs::path> { };

TEST_P(ParseTreeWriterGolden, MatchesLegacyXml) {
  const fs::path path = GetParam();
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
                         testing::ValuesIn(fluir::test::getTestPrograms("write")),
                         fluir::test::filePathName);

TEST(ParseTreeWriter, WritesEmptyFunctionFromHandBuiltTree) {
  fluir::editor::et::ParseTree tree;
  tree.header.version = {0, 1, 3};
  fluir::editor::et::FunctionDecl fn;
  fn.id = 1;
  fn.location = {.x = 10, .y = 10, .z = 3, .width = 100, .height = 100};
  fn.name = "foo";
  tree.declarations.emplace(fn.id, fluir::editor::et::Declaration{fn});

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
  fluir::editor::et::ParseTree edited = *loaded.tree;
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

namespace {

  // Writes `tree` and parses the result back, as saving and reopening a file does.
  fluir::editor::LoadResult roundTrip(fluir::editor::CollectingSink& sink, const fluir::editor::et::ParseTree& tree) {
    std::ostringstream out;
    fluir::editor::ParseTreeWriter(out).write(tree);
    return parse(sink, out.str());
  }

  const fluir::editor::et::Conditional& conditionalOf(const fluir::editor::et::ParseTree& tree,
                                                      fluir::ID fn,
                                                      fluir::ID id) {
    return std::get<fluir::editor::et::Conditional>(
      std::get<fluir::editor::et::FunctionDecl>(tree.declarations.at(fn)).body.nodes.at(id));
  }

}  // namespace

TEST(ParseTreeWriter, AConditionalWithEmptyBranchesSurvivesARoundTrip) {
  fluir::editor::CollectingSink sink;
  const auto loaded = parse(sink, readFile(fs::path(TEST_FOLDER) / "read/conditional_empty_scopes.fl"));
  ASSERT_TRUE(loaded.tree.has_value());

  fluir::editor::CollectingSink reloadSink;
  const auto reloaded = roundTrip(reloadSink, *loaded.tree);

  ASSERT_TRUE(reloaded.tree.has_value());
  EXPECT_EQ(*reloaded.tree, *loaded.tree);
}

// The writer drops a binary's lhs/rhs, which this fixture sets, so the trees are not equal
// whole. What nesting must preserve is the branches' contents.
TEST(ParseTreeWriter, BranchesAndTheirContentsSurviveARoundTrip) {
  fluir::editor::CollectingSink sink;
  const auto loaded = parse(sink, readFile(fs::path(TEST_FOLDER) / "read/conditional_with_body.fl"));
  ASSERT_TRUE(loaded.tree.has_value());

  fluir::editor::CollectingSink reloadSink;
  const auto reloaded = roundTrip(reloadSink, *loaded.tree);
  ASSERT_TRUE(reloaded.tree.has_value());

  const fluir::editor::et::Conditional& before = conditionalOf(*loaded.tree, 1, 2);
  const fluir::editor::et::Conditional& after = conditionalOf(*reloaded.tree, 1, 2);
  EXPECT_EQ(after.location, before.location);
  EXPECT_EQ(after.condition, before.condition);
  for (const auto& [b, a] :
       {std::pair{&*before.thenScope, &*after.thenScope}, std::pair{&*before.elseScope, &*after.elseScope}}) {
    EXPECT_EQ(a->conduits, b->conduits);
    ASSERT_EQ(a->nodes.size(), b->nodes.size());
    for (const auto& [id, node] : b->nodes) {
      ASSERT_TRUE(a->nodes.contains(id));
      EXPECT_EQ(a->nodes.at(id).index(), node.index()) << "node " << id << " changed kind";
    }
  }
}
