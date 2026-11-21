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

}  // namespace
class TestMainWindow : public ::testing::Test {
 public:
  fluir::editor::EditorContext ctx;
  MainWindow window{ctx};

  QAction* action(const char* name) {
    auto* found = window.findChild<QAction*>(name);
    EXPECT_NE(found, nullptr) << name;
    return found;
  }

  ModulePage* page() { return window.findChild<ModulePage*>(); }
};

TEST_F(TestMainWindow, StartsOnTheSplashWithNoPage) { EXPECT_EQ(page(), nullptr); }

TEST_F(TestMainWindow, TheSplashHasNoModuleActions) {
  for (const char* name : {"save", "saveAs", "undo", "redo", "exit"}) {
    EXPECT_EQ(window.findChild<QAction*>(name), nullptr) << name;
  }
}

TEST_F(TestMainWindow, TheSplashNewButtonOpensAnUnsavedModule) {
  QTest::mouseClick(window.findChild<QPushButton*>("new"), Qt::LeftButton);

  ASSERT_NE(page(), nullptr);
  EXPECT_FALSE(page()->program().has_value());
  EXPECT_TRUE(page()->isVisibleTo(&window));
  EXPECT_TRUE(window.windowTitle().contains("<new file>"));
}

TEST_F(TestMainWindow, ThePageStaysAChildInsideTheStack) {
  QTest::mouseClick(window.findChild<QPushButton*>("new"), Qt::LeftButton);

  ASSERT_NE(page(), nullptr);
  EXPECT_FALSE(page()->isWindow());
  EXPECT_EQ(window.findChild<QStackedWidget*>()->currentWidget(), page());
}

TEST_F(TestMainWindow, OpenModuleShowsTheProgram) {
  ASSERT_TRUE(window.openModule(kIntConstants));

  ASSERT_NE(page(), nullptr);
  EXPECT_EQ(page()->program(), kIntConstants);
  EXPECT_TRUE(window.windowTitle().contains("int_constants.fl"));
  EXPECT_TRUE(page()->isVisibleTo(&window));
}

TEST_F(TestMainWindow, AFileThatFailsToLoadStaysOnTheSplash) {
  EXPECT_FALSE(window.openModule(fs::path(TEST_FOLDER) / "load_fails/conduits.fl"));

  EXPECT_EQ(page(), nullptr);
}

TEST_F(TestMainWindow, AFileThatFailsToBuildStaysOnTheSplash) {
  EXPECT_FALSE(window.openModule(fs::path(TEST_FOLDER) / "build_fails/dangling_sink.fl"));

  EXPECT_EQ(page(), nullptr);
}

TEST_F(TestMainWindow, ANewPageStartsWithNoHistoryToUndo) {
  ASSERT_TRUE(window.openModule(kIntConstants));
  page()->undoStack().push(new NoopCommand);

  ASSERT_TRUE(window.openModule(kIntConstants));
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

  ASSERT_NE(page(), nullptr);
  EXPECT_FALSE(page()->undoStack().canUndo());
}

TEST_F(TestMainWindow, TheTitleFollowsSaveAs) {
  QTest::mouseClick(window.findChild<QPushButton*>("new"), Qt::LeftButton);
  ASSERT_NE(page(), nullptr);
  const fs::path out = fs::temp_directory_path() / "fluir_window_test_save_as.fl";

  ASSERT_TRUE(page()->saveAs(out));

  EXPECT_TRUE(window.windowTitle().contains("fluir_window_test_save_as.fl"));
}

TEST_F(TestMainWindow, ExitReturnsToTheSplash) {
  ASSERT_TRUE(window.openModule(kIntConstants));

  const QPointer<ModulePage> p = page();
  ASSERT_NE(p, nullptr);

  action("exit")->trigger();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

  EXPECT_EQ(p, nullptr);
  EXPECT_EQ(page(), nullptr);
  EXPECT_EQ(window.windowTitle(), "Fluir");
}

TEST(TestMainWindowProgramPage, CanOpenModulePageDirectly) {
  // No fixture because we need to construct a different window directly
  fluir::editor::EditorContext ctx;
  auto window = MainWindow{ctx, kIntConstants};

  auto* page = window.findChild<ModulePage*>();

  ASSERT_NE(page, nullptr);
  EXPECT_EQ(page->program(), kIntConstants);
  EXPECT_TRUE(window.windowTitle().contains("int_constants.fl"));
  EXPECT_TRUE(page->isVisibleTo(&window));
}
