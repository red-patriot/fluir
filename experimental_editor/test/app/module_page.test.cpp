#include "editor/app/module_page.hpp"

#include <filesystem>
#include <optional>
#include <QAction>
#include <QKeySequence>
#include <QSignalSpy>
#include <QUndoCommand>
#include <QUndoStack>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/scene/graph_view.hpp"
#include "editor/scene/module_scene.hpp"
#include "fixture_loader.hpp"

namespace {

  namespace fs = std::filesystem;

  using fluir::editor::ModulePage;

  const fs::path kIntConstants = fs::path(TEST_FOLDER) / "read/int_constants.fl";

  fluir::pt::ParseTree load(const fs::path& program) {
    testutil::Loaded loaded = testutil::loadFixture(program.string());
    EXPECT_TRUE(loaded.result.tree.has_value());
    return std::move(loaded.result.tree).value_or(fluir::pt::ParseTree{});
  }

  fs::path scratchCopy(const fs::path& fixture, const std::string& name) {
    const fs::path out = fs::temp_directory_path() / ("fluir_page_test_" + name + ".fl");
    fs::copy_file(fixture, out, fs::copy_options::overwrite_existing);
    return out;
  }

  // Counts its runs; stands in for an edit until the commands land.
  struct CountingCommand : QUndoCommand {
    int* runs;
    explicit CountingCommand(int* r) : runs(r) { }
    void redo() override { ++*runs; }
    void undo() override { --*runs; }
  };

  struct Harness {
    fluir::editor::EditorContext ctx;
    ModulePage page;

    Harness(std::optional<fs::path> program, fluir::pt::ParseTree tree) :
      page(ctx, std::move(program), std::move(tree)) { }

    explicit Harness(const fs::path& program) : Harness(program, load(program)) { }

    QAction* action(const char* name) {
      auto* found = page.findChild<QAction*>(name);
      EXPECT_NE(found, nullptr) << name;
      return found;
    }
  };

}  // namespace

TEST(ModulePage, ActionsCarryTheStandardShortcuts) {
  Harness h(kIntConstants);

  EXPECT_EQ(h.action("save")->shortcut(), QKeySequence(QKeySequence::Save));
  EXPECT_EQ(h.action("saveAs")->shortcut(), QKeySequence(QKeySequence::SaveAs));
  EXPECT_EQ(h.action("undo")->shortcut(), QKeySequence(QKeySequence::Undo));
  EXPECT_EQ(h.action("redo")->shortcut(), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z));
}

TEST(ModulePage, UndoAndRedoFollowTheStack) {
  Harness h(kIntConstants);
  int runs = 0;
  EXPECT_FALSE(h.action("undo")->isEnabled());
  EXPECT_FALSE(h.action("redo")->isEnabled());

  h.page.undoStack().push(new CountingCommand(&runs));
  ASSERT_TRUE(h.action("undo")->isEnabled());
  EXPECT_FALSE(h.action("redo")->isEnabled());

  h.action("undo")->trigger();
  EXPECT_EQ(runs, 0);
  EXPECT_FALSE(h.action("undo")->isEnabled());
  ASSERT_TRUE(h.action("redo")->isEnabled());

  h.action("redo")->trigger();
  EXPECT_EQ(runs, 1);
}

TEST(ModulePage, ShowsItsSceneInTheView) {
  Harness h(kIntConstants);

  const auto* view = h.page.findChild<fluir::editor::GraphView*>();
  ASSERT_NE(view, nullptr);
  EXPECT_NE(dynamic_cast<fluir::editor::ModuleScene*>(view->scene()), nullptr);
}

TEST(ModulePage, SaveWritesTheModuleToTheProgram) {
  const fs::path program = scratchCopy(kIntConstants, "save");
  const fluir::pt::ParseTree tree = load(program);
  Harness h(program, tree);
  fs::resize_file(program, 0);

  h.action("save")->trigger();

  const testutil::Loaded reloaded = testutil::loadFixture(program.string());
  ASSERT_TRUE(reloaded.result.tree.has_value());
  EXPECT_EQ(*reloaded.result.tree, tree);
}

TEST(ModulePage, SaveMarksTheStackClean) {
  const fs::path program = scratchCopy(kIntConstants, "clean");
  Harness h(program);
  int runs = 0;
  h.page.undoStack().push(new CountingCommand(&runs));
  ASSERT_FALSE(h.page.undoStack().isClean());

  h.action("save")->trigger();

  EXPECT_TRUE(h.page.undoStack().isClean());
}

TEST(ModulePage, SaveAsAddsTheExtensionAndMovesTheProgram) {
  Harness h(std::nullopt, fluir::pt::ParseTree{});
  QSignalSpy moved(&h.page, &ModulePage::programChanged);
  const fs::path out = fs::temp_directory_path() / "fluir_page_test_save_as";
  const fs::path written = fs::path(out).replace_extension("fl");
  fs::remove(written);

  ASSERT_TRUE(h.page.saveAs(out));

  EXPECT_TRUE(fs::exists(written));
  EXPECT_EQ(h.page.program(), written);
  EXPECT_EQ(moved.count(), 1);
}

TEST(ModulePage, ExitRequestsClose) {
  Harness h(kIntConstants);
  QSignalSpy closes(&h.page, &ModulePage::closeRequested);

  h.action("exit")->trigger();

  EXPECT_EQ(closes.count(), 1);
}
