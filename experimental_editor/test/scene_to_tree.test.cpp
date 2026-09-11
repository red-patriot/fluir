#include "editor/core/scene_to_tree.hpp"

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/actors/scene.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/parse_tree_writer.hpp"
#include "fixture_loader.hpp"

// The scene is the edit model; the parse tree is only a save format. These tests
// pin that regenerating the tree from the scene loses nothing the writer emits.

namespace {

  namespace fs = std::filesystem;

  using testutil::Loaded;

  const fluir::editor::EditorContext kCtx;

  std::string writeToString(const fluir::pt::ParseTree& tree) {
    std::ostringstream out;
    fluir::editor::ParseTreeWriter(out).write(tree);
    return out.str();
  }

  // Every fixture under test/programs, in a stable order.
  std::vector<fs::path> fixtures() {
    std::vector<fs::path> paths;
    for (const char* dir : {"read", "write"}) {
      for (const auto& entry : fs::recursive_directory_iterator(fs::path(TEST_FOLDER) / dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".fl") {
          paths.push_back(entry.path());
        }
      }
    }
    std::sort(paths.begin(), paths.end());
    return paths;
  }

}  // namespace

class SceneToTreeFidelity : public testing::TestWithParam<fs::path> { };

TEST_P(SceneToTreeFidelity, WritingFromTheSceneMatchesWritingFromTheLoadedTree) {
  Loaded l;
  fluir::Context ctx{
    .diagnosticSink = l.sink,
    .symbolTable = {},
    .currentFile = {},
    .outputFilename = {},
    .version = {},
    .ignoreVersionChecks = true,
  };
  l.result = fluir::editor::loadFile(ctx, GetParam());
  if (!l.result.tree) {
    GTEST_SKIP() << "fixture does not load: " << GetParam().string();
  }

  fluir::editor::GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  EXPECT_EQ(writeToString(fluir::editor::sceneToParseTree(scene, l.result.tree->header)),
            writeToString(*l.result.tree));
}

INSTANTIATE_TEST_SUITE_P(Fixtures, SceneToTreeFidelity, testing::ValuesIn(fixtures()), [](const auto& i) {
  return i.param.stem().string();
});
