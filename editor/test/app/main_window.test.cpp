#include "editor/app/main_window.hpp"

#include <filesystem>
#include <QAction>
#include <QCoreApplication>
#include <QEvent>
#include <QPointer>
#include <QPushButton>
#include <QStackedWidget>
#include <QTest>
#include <QUndoCommand>
#include <QUndoStack>
#include <string>

#include <gtest/gtest.h>

#include "editor/app/module_page.hpp"
#include "editor/core/editor_context.hpp"

namespace {

  namespace fs = std::filesystem;

  using fluir::editor::MainWindow;
  using fluir::editor::ModulePage;

  const fs::path kIntConstants = fs::path(TEST_FOLDER) / "read/int_constants.fl";

  // Stands in for an edit until the commands land.
  struct NoopCommand : QUndoCommand {
    void redo() override { }
    void undo() override { }
  };

  struct Harness {
    fluir::editor::EditorContext ctx;
    MainWindow window{ctx};

    QAction* action(const char* name) {
      auto* found = window.findChild<QAction*>(name);
      EXPECT_NE(found, nullptr) << name;
      return found;
    }

    ModulePage* page() { return window.findChild<ModulePage*>(); }
  };

}  // namespace

TEST(MainWindow, StartsOnTheSplashWithNoPage) {
  Harness h;

  EXPECT_EQ(h.page(), nullptr);
}

TEST(MainWindow, TheSplashHasNoModuleActions) {
  Harness h;

  for (const char* name : {"save", "saveAs", "undo", "redo", "exit"}) {
    EXPECT_EQ(h.window.findChild<QAction*>(name), nullptr) << name;
  }
}

TEST(MainWindow, TheSplashNewButtonOpensAnUnsavedModule) {
  Harness h;

  QTest::mouseClick(h.window.findChild<QPushButton*>("new"), Qt::LeftButton);

  ASSERT_NE(h.page(), nullptr);
  EXPECT_FALSE(h.page()->program().has_value());
  EXPECT_TRUE(h.page()->isVisibleTo(&h.window));
  EXPECT_TRUE(h.window.windowTitle().contains("<new file>"));
}

TEST(MainWindow, ThePageStaysAChildInsideTheStack) {
  Harness h;

  QTest::mouseClick(h.window.findChild<QPushButton*>("new"), Qt::LeftButton);

  ASSERT_NE(h.page(), nullptr);
  EXPECT_FALSE(h.page()->isWindow());
  EXPECT_EQ(h.window.findChild<QStackedWidget*>()->currentWidget(), h.page());
}

TEST(MainWindow, OpenModuleShowsTheProgram) {
  Harness h;

  ASSERT_TRUE(h.window.openModule(kIntConstants));

  ASSERT_NE(h.page(), nullptr);
  EXPECT_EQ(h.page()->program(), kIntConstants);
  EXPECT_TRUE(h.window.windowTitle().contains("int_constants.fl"));
  EXPECT_TRUE(h.page()->isVisibleTo(&h.window));
}

TEST(MainWindow, AFileThatFailsToLoadStaysOnTheSplash) {
  Harness h;

  EXPECT_FALSE(h.window.openModule(fs::path(TEST_FOLDER) / "load_fails/conduits.fl"));

  EXPECT_EQ(h.page(), nullptr);
}

TEST(MainWindow, AFileThatFailsToBuildStaysOnTheSplash) {
  Harness h;

  EXPECT_FALSE(h.window.openModule(fs::path(TEST_FOLDER) / "build_fails/dangling_sink.fl"));

  EXPECT_EQ(h.page(), nullptr);
}

TEST(MainWindow, ANewPageStartsWithNoHistoryToUndo) {
  Harness h;
  ASSERT_TRUE(h.window.openModule(kIntConstants));
  h.page()->undoStack().push(new NoopCommand);

  ASSERT_TRUE(h.window.openModule(kIntConstants));
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

  ASSERT_NE(h.page(), nullptr);
  EXPECT_FALSE(h.page()->undoStack().canUndo());
}

TEST(MainWindow, TheTitleFollowsSaveAs) {
  Harness h;
  QTest::mouseClick(h.window.findChild<QPushButton*>("new"), Qt::LeftButton);
  ASSERT_NE(h.page(), nullptr);
  const fs::path out = fs::temp_directory_path() / "fluir_window_test_save_as.fl";

  ASSERT_TRUE(h.page()->saveAs(out));

  EXPECT_TRUE(h.window.windowTitle().contains("fluir_window_test_save_as.fl"));
}

TEST(MainWindow, ExitReturnsToTheSplash) {
  Harness h;
  ASSERT_TRUE(h.window.openModule(kIntConstants));

  const QPointer<ModulePage> page = h.page();
  ASSERT_NE(page, nullptr);

  h.action("exit")->trigger();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

  EXPECT_EQ(page, nullptr);
  EXPECT_EQ(h.page(), nullptr);
  EXPECT_EQ(h.window.windowTitle(), "Fluir");
}
