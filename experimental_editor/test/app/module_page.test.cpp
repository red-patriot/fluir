#include "editor/app/module_page.hpp"

#include <filesystem>
#include <optional>
#include <QAction>
#include <QKeySequence>
#include <QSignalSpy>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/module_editor.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/scene/elements/element.hpp"
#include "editor/scene/graph_scene.hpp"
#include "editor/scene/graph_view.hpp"
#include "editor/transaction/move.hpp"
#include "fixture_loader.hpp"

namespace {

  namespace fs = std::filesystem;

  using fluir::editor::ModuleEditor;
  using fluir::editor::ModulePage;
  using fluir::editor::moveTo;

  const fs::path kIntConstants = fs::path(TEST_FOLDER) / "read/int_constants.fl";
  // int_constants.fl: constant 1 of function 1 sits at units (2, 20).
  const fluir::FullID kConstant{1, 1};

  ModuleEditor open(const fs::path& program) {
    std::optional<ModuleEditor> editor = fluir::editor::openModule(program);
    EXPECT_TRUE(editor.has_value());
    return std::move(editor).value_or(fluir::editor::newModule());
  }

  fs::path scratchCopy(const fs::path& fixture, const std::string& name) {
    const fs::path out = fs::temp_directory_path() / ("fluir_page_test_" + name + ".fl");
    fs::copy_file(fixture, out, fs::copy_options::overwrite_existing);
    return out;
  }

  struct Harness {
    fluir::editor::EditorContext ctx;
    ModulePage page;

    explicit Harness(ModuleEditor editor) : page(ctx, std::move(editor)) { }

    QAction* action(const char* name) {
      auto* found = page.findChild<QAction*>(name);
      EXPECT_NE(found, nullptr) << name;
      return found;
    }

    fluir::editor::GraphScene* scene() {
      return dynamic_cast<fluir::editor::GraphScene*>(page.findChild<fluir::editor::GraphView*>()->scene());
    }

    int x() { return fluir::editor::locationAt(page.document().editor().tree(), kConstant)->x; }
  };

}  // namespace

TEST(ModulePage, ActionsCarryTheStandardShortcuts) {
  Harness h(open(kIntConstants));

  EXPECT_EQ(h.action("save")->shortcut(), QKeySequence(QKeySequence::Save));
  EXPECT_EQ(h.action("saveAs")->shortcut(), QKeySequence(QKeySequence::SaveAs));
  EXPECT_EQ(h.action("undo")->shortcut(), QKeySequence(QKeySequence::Undo));
  EXPECT_EQ(h.action("redo")->shortcut(), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z));
}

TEST(ModulePage, UndoAndRedoFollowTheHistory) {
  Harness h(open(kIntConstants));
  EXPECT_FALSE(h.action("undo")->isEnabled());
  EXPECT_FALSE(h.action("redo")->isEnabled());

  h.page.document().apply(moveTo(kConstant, 4, 20));
  ASSERT_TRUE(h.action("undo")->isEnabled());
  EXPECT_FALSE(h.action("redo")->isEnabled());

  h.action("undo")->trigger();
  EXPECT_EQ(h.x(), 2);
  EXPECT_FALSE(h.action("undo")->isEnabled());
  ASSERT_TRUE(h.action("redo")->isEnabled());

  h.action("redo")->trigger();
  EXPECT_EQ(h.x(), 4);
}

TEST(ModulePage, SaveWritesTheEditedTreeToTheProgram) {
  const fs::path program = scratchCopy(kIntConstants, "save");
  Harness h(open(program));
  h.page.document().apply(moveTo(kConstant, 4, 20));

  h.action("save")->trigger();

  const testutil::Loaded reloaded = testutil::loadFixture(program.string());
  ASSERT_TRUE(reloaded.result.tree.has_value());
  EXPECT_EQ(*reloaded.result.tree, h.page.document().editor().tree());
}

TEST(ModulePage, SaveAsAddsTheExtension) {
  Harness h(fluir::editor::newModule());
  const fs::path out = fs::temp_directory_path() / "fluir_page_test_save_as";
  const fs::path written = fs::path(out).replace_extension("fl");
  fs::remove(written);

  ASSERT_TRUE(h.page.saveAs(out));

  EXPECT_TRUE(fs::exists(written));
  EXPECT_EQ(h.page.document().editor().program(), written);
}

TEST(ModulePage, ShowsTheModuleAsItsScene) {
  Harness h(open(kIntConstants));

  ASSERT_NE(h.scene(), nullptr);
  EXPECT_NE(h.scene()->itemAt(kConstant), nullptr);
}

TEST(ModulePage, AnEditReachesTheScene) {
  Harness h(open(kIntConstants));
  ASSERT_NE(h.scene(), nullptr);
  ASSERT_NE(h.scene()->itemAt(kConstant), nullptr);
  const QRectF before = h.scene()->itemAt(kConstant)->sceneBoundingRect();

  h.page.document().apply(moveTo(kConstant, 4, 20));

  EXPECT_NE(h.scene()->itemAt(kConstant)->sceneBoundingRect(), before);
}

TEST(ModulePage, ExitRequestsClose) {
  Harness h(open(kIntConstants));
  QSignalSpy closes(&h.page, &ModulePage::closeRequested);

  h.action("exit")->trigger();

  EXPECT_EQ(closes.count(), 1);
}
