#include "editor/app/main_window.hpp"

#include <filesystem>
#include <QAction>
#include <QCoreApplication>
#include <QEvent>
#include <QPointer>
#include <QPushButton>
#include <QTest>
#include <string>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "editor/app/module_document.hpp"
#include "editor/app/module_page.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/transaction/move.hpp"

namespace {

  namespace fs = std::filesystem;

  using fluir::editor::MainWindow;
  using fluir::editor::ModulePage;
  using fluir::editor::moveTo;

  const fs::path kIntConstants = fs::path(TEST_FOLDER) / "read/int_constants.fl";
  // int_constants.fl: constant 1 of function 1 sits at units (2, 20).
  const fluir::FullID kConstant{1, 1};

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

TEST(MainWindow, StartsOnTheSplashWithNoDocument) {
  Harness h;

  EXPECT_EQ(h.window.document(), nullptr);
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

  ASSERT_NE(h.window.document(), nullptr);
  EXPECT_FALSE(h.window.document()->editor().program().has_value());
  ASSERT_NE(h.page(), nullptr);
  EXPECT_TRUE(h.page()->isVisibleTo(&h.window));
  EXPECT_TRUE(h.window.windowTitle().contains("<new file>"));
}

TEST(MainWindow, OpenModuleShowsTheProgram) {
  Harness h;

  ASSERT_TRUE(h.window.openModule(kIntConstants));

  ASSERT_NE(h.window.document(), nullptr);
  EXPECT_EQ(h.window.document()->editor().program(), kIntConstants);
  EXPECT_TRUE(h.window.windowTitle().contains("int_constants.fl"));
  ASSERT_NE(h.page(), nullptr);
  EXPECT_TRUE(h.page()->isVisibleTo(&h.window));
}

TEST(MainWindow, AFileThatFailsToLoadStaysOnTheSplash) {
  Harness h;

  EXPECT_FALSE(h.window.openModule(fs::path(TEST_FOLDER) / "load_fails/conduits.fl"));

  EXPECT_EQ(h.window.document(), nullptr);
  EXPECT_EQ(h.page(), nullptr);
}

TEST(MainWindow, ANewDocumentStartsWithNoHistoryToUndo) {
  Harness h;
  ASSERT_TRUE(h.window.openModule(kIntConstants));
  h.window.document()->apply(moveTo(kConstant, 4, 20));

  ASSERT_TRUE(h.window.openModule(kIntConstants));

  EXPECT_FALSE(h.window.document()->editor().canUndo());
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
  EXPECT_EQ(h.window.document(), nullptr);
  EXPECT_EQ(h.window.windowTitle(), "Fluir");
}
