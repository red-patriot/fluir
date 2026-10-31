#include "editor/diagram/export.hpp"

#include <filesystem>
#include <QUndoStack>
#include <sstream>

#include <gtest/gtest.h>

#include "bytecode/version.hpp"
#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/parse_tree_writer.hpp"
#include "editor/diagram/build.hpp"
#include "editor/scene/module_scene.hpp"
#include "file_utility.hpp"
#include "fixture_loader.hpp"

namespace {

  namespace fs = std::filesystem;
  namespace diagram = fluir::editor::diagram;

  struct Harness {
    fluir::editor::EditorContext ctx;
    QUndoStack stack;
    fluir::editor::ModuleScene scene{ctx, stack};
  };

  fluir::pt::ParseTree load(const fs::path& fixture) {
    testutil::Loaded loaded = testutil::loadFixture(fixture.string());
    EXPECT_TRUE(loaded.result.tree.has_value()) << fixture;
    return std::move(loaded.result.tree).value_or(fluir::pt::ParseTree{});
  }

  std::vector<fs::path> parseableWriteFixtures() {
    std::vector<fs::path> out;
    for (const fs::path& path : testutil::readWriteFixtures(true)) {
      if (path.parent_path().filename() == "write") {
        out.push_back(path);
      }
    }
    return out;
  }

}  // namespace

class ExportEachFixture : public testing::TestWithParam<fs::path> { };

INSTANTIATE_TEST_SUITE_P(Fixtures,
                         ExportEachFixture,
                         testing::ValuesIn(testutil::readWriteFixtures(true)),
                         testutil::fixtureName);

TEST_P(ExportEachFixture, ExportsTheTreeItWasBuiltFrom) {
  Harness h;
  const fluir::pt::ParseTree tree = load(GetParam());
  ASSERT_TRUE(diagram::buildScene(tree, h.scene));

  EXPECT_EQ(diagram::exportTree(h.scene), tree);
}

class SaveEachWriteFixture : public testing::TestWithParam<fs::path> { };

INSTANTIATE_TEST_SUITE_P(Fixtures,
                         SaveEachWriteFixture,
                         testing::ValuesIn(parseableWriteFixtures()),
                         testutil::fixtureName);

TEST_P(SaveEachWriteFixture, WritesTheFileItWasBuiltFrom) {
  Harness h;
  ASSERT_TRUE(diagram::buildScene(load(GetParam()), h.scene));

  std::ostringstream out;
  fluir::editor::ParseTreeWriter(out).write(diagram::exportTree(h.scene));

  EXPECT_EQ(out.str(), fluir::test::readContents(GetParam()));
}

TEST(Export, AnEmptyModuleKeepsItsHeader) {
  Harness h;
  fluir::pt::ParseTree tree;
  tree.header.version = fluir::CURRENT_VERSION;
  ASSERT_TRUE(diagram::buildScene(tree, h.scene));

  EXPECT_EQ(diagram::exportTree(h.scene), tree);
}
