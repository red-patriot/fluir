#include "editor/app/main_window.hpp"

#include <filesystem>
#include <QAction>
#include <QKeySequence>
#include <QPushButton>
#include <QTest>
#include <string>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "editor/app/module_document.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/scene/graph_view.hpp"
#include "editor/transaction/move.hpp"
#include "fixture_loader.hpp"

namespace {

  namespace fs = std::filesystem;

  using fluir::editor::MainWindow;
  using fluir::editor::moveTo;

  const fs::path kIntConstants = fs::path(TEST_FOLDER) / "read/int_constants.fl";
  // int_constants.fl: constant 1 of function 1 sits at units (2, 20).
  const fluir::FullID kConstant{1, 1};

  fs::path scratchCopy(const fs::path& fixture, const std::string& name) {
    const fs::path out = fs::temp_directory_path() / ("fluir_window_test_" + name + ".fl");
    fs::copy_file(fixture, out, fs::copy_options::overwrite_existing);
    return out;
  }

  struct Harness {
    fluir::editor::EditorContext ctx;
    MainWindow window{ctx};

    QAction* action(const char* name) {
      auto* found = window.findChild<QAction*>(name);
      EXPECT_NE(found, nullptr) << name;
      return found;
    }

    int x() const { return fluir::editor::locationAt(window.document()->editor().tree(), kConstant)->x; }
  };

}  // namespace

TEST(MainWindow, StartsOnTheSplashWithNoDocument) {
  Harness h;

  EXPECT_EQ(h.window.document(), nullptr);
  EXPECT_FALSE(h.window.findChild<fluir::editor::GraphView*>()->isVisibleTo(&h.window));
  for (const char* name : {"save", "saveAs", "undo", "redo", "exit"}) {
    EXPECT_FALSE(h.action(name)->isEnabled()) << name;
  }
}

TEST(MainWindow, TheSplashNewButtonOpensAnUnsavedModule) {
  Harness h;

  QTest::mouseClick(h.window.findChild<QPushButton*>("new"), Qt::LeftButton);

  ASSERT_NE(h.window.document(), nullptr);
  EXPECT_FALSE(h.window.document()->editor().program().has_value());
  EXPECT_TRUE(h.window.findChild<fluir::editor::GraphView*>()->isVisibleTo(&h.window));
  EXPECT_TRUE(h.window.windowTitle().contains("<new file>"));
}

TEST(MainWindow, OpenModuleShowsTheProgram) {
  Harness h;

  ASSERT_TRUE(h.window.openModule(kIntConstants));

  ASSERT_NE(h.window.document(), nullptr);
  EXPECT_EQ(h.window.document()->editor().program(), kIntConstants);
  EXPECT_TRUE(h.window.windowTitle().contains("int_constants.fl"));
  EXPECT_TRUE(h.action("save")->isEnabled());
}

TEST(MainWindow, AFileThatFailsToLoadStaysOnTheSplash) {
  Harness h;

  EXPECT_FALSE(h.window.openModule(fs::path(TEST_FOLDER) / "load_fails/conduits.fl"));

  EXPECT_EQ(h.window.document(), nullptr);
}

TEST(MainWindow, ActionsCarryTheStandardShortcuts) {
  Harness h;

  EXPECT_EQ(h.action("save")->shortcut(), QKeySequence(QKeySequence::Save));
  EXPECT_EQ(h.action("saveAs")->shortcut(), QKeySequence(QKeySequence::SaveAs));
  EXPECT_EQ(h.action("undo")->shortcut(), QKeySequence(QKeySequence::Undo));
  EXPECT_EQ(h.action("redo")->shortcut(), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z));
}

TEST(MainWindow, UndoAndRedoFollowTheHistory) {
  Harness h;
  ASSERT_TRUE(h.window.openModule(kIntConstants));
  EXPECT_FALSE(h.action("undo")->isEnabled());

  h.window.document()->apply(moveTo(kConstant, 4, 20));
  ASSERT_TRUE(h.action("undo")->isEnabled());
  EXPECT_FALSE(h.action("redo")->isEnabled());

  h.action("undo")->trigger();
  EXPECT_EQ(h.x(), 2);
  EXPECT_FALSE(h.action("undo")->isEnabled());
  ASSERT_TRUE(h.action("redo")->isEnabled());

  h.action("redo")->trigger();
  EXPECT_EQ(h.x(), 4);
}

TEST(MainWindow, ANewDocumentStartsWithNoHistoryToUndo) {
  Harness h;
  ASSERT_TRUE(h.window.openModule(kIntConstants));
  h.window.document()->apply(moveTo(kConstant, 4, 20));

  ASSERT_TRUE(h.window.openModule(kIntConstants));

  EXPECT_FALSE(h.action("undo")->isEnabled());
}

TEST(MainWindow, SaveWritesTheEditedTreeToTheProgram) {
  Harness h;
  const fs::path program = scratchCopy(kIntConstants, "save");
  ASSERT_TRUE(h.window.openModule(program));
  h.window.document()->apply(moveTo(kConstant, 4, 20));

  h.action("save")->trigger();

  const testutil::Loaded reloaded = testutil::loadFixture(program.string());
  ASSERT_TRUE(reloaded.result.tree.has_value());
  EXPECT_EQ(*reloaded.result.tree, h.window.document()->editor().tree());
}

TEST(MainWindow, SaveModuleAsAddsTheExtensionAndRetitles) {
  Harness h;
  QTest::mouseClick(h.window.findChild<QPushButton*>("new"), Qt::LeftButton);
  const fs::path out = fs::temp_directory_path() / "fluir_window_test_save_as";
  fs::remove(fs::path(out).replace_extension("fl"));

  ASSERT_TRUE(h.window.saveModuleAs(out));

  EXPECT_TRUE(fs::exists(fs::path(out).replace_extension("fl")));
  EXPECT_TRUE(h.window.windowTitle().contains("fluir_window_test_save_as.fl"));
}

TEST(MainWindow, ExitReturnsToTheSplash) {
  Harness h;
  ASSERT_TRUE(h.window.openModule(kIntConstants));

  h.action("exit")->trigger();

  EXPECT_EQ(h.window.document(), nullptr);
  EXPECT_FALSE(h.window.findChild<fluir::editor::GraphView*>()->isVisibleTo(&h.window));
  EXPECT_FALSE(h.action("undo")->isEnabled());
}
