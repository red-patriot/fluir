#include <filesystem>
#include <QAction>
#include <QPointer>
#include <QPushButton>
#include <QTest>
#include <string>

#include <gtest/gtest.h>

#include "editor/app/main_window.hpp"
#include "editor/app/module_page.hpp"
#include "editor/core/editor_context.hpp"

namespace {
  namespace fs = std::filesystem;

  using fluir::editor::MainWindow;
  using fluir::editor::ModulePage;

  const fs::path INT_CONSTANTS_FL = fs::path(TEST_FOLDER) / "read/int_constants.fl";
}  // namespace

class TestMainWindowModulePage : public ::testing::Test {
 public:
  fluir::editor::EditorContext ctx;
  MainWindow window{ctx, INT_CONSTANTS_FL};

  QAction* action(const char* name) {
    auto* found = window.findChild<QAction*>(name);
    EXPECT_NE(found, nullptr) << name;
    return found;
  }

  ModulePage* page() { return window.findChild<ModulePage*>(); }
};

TEST_F(TestMainWindowModulePage, CanOpenModulePageDirectly) {
  ASSERT_NE(page(), nullptr);
  EXPECT_EQ(page()->program(), INT_CONSTANTS_FL);
  EXPECT_TRUE(window.windowTitle().contains("int_constants.fl"));
  EXPECT_TRUE(page()->isVisibleTo(&window));
}

TEST_F(TestMainWindowModulePage, CanGoBackToSplash) {
  const QPointer<ModulePage> p = page();
  ASSERT_NE(p, nullptr);

  action("exit")->trigger();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

  EXPECT_EQ(p, nullptr);
  EXPECT_EQ(page(), nullptr);
  EXPECT_EQ(window.windowTitle(), "Fluir");
}
