#include "editor/app/module_document.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <QSignalSpy>
#include <string>
#include <utility>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "editor/core/module_editor.hpp"
#include "editor/core/tree.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/move.hpp"
#include "fixture_loader.hpp"

namespace {

  namespace fs = std::filesystem;

  using fluir::editor::ModuleDocument;
  using fluir::editor::moveTo;

  // simple_binary_expr.fl: constant 2 of function 1 sits at (2, 2).
  const fluir::FullID kConstant{1, 2};

  fluir::editor::ModuleEditor openFixture(const std::string& relPath) {
    std::optional<fluir::editor::ModuleEditor> editor = fluir::editor::openModule(fs::path(TEST_FOLDER) / relPath);
    EXPECT_TRUE(editor.has_value());
    return std::move(editor).value_or(fluir::editor::newModule());
  }

  fs::path scratchCopy(const std::string& relPath, const std::string& name) {
    const fs::path out = fs::temp_directory_path() / ("fluir_document_test_" + name + ".fl");
    fs::copy_file(fs::path(TEST_FOLDER) / relPath, out, fs::copy_options::overwrite_existing);
    return out;
  }

}  // namespace

TEST(ModuleDocument, ApplyAnnouncesTheChange) {
  ModuleDocument uut{openFixture("read/simple_binary_expr.fl")};
  QSignalSpy changed(&uut, &ModuleDocument::changed);

  EXPECT_TRUE(uut.apply(moveTo(kConstant, 7, 9)));

  EXPECT_EQ(changed.count(), 1);
  EXPECT_EQ(fluir::editor::locationAt(uut.editor().tree(), kConstant)->x, 7);
}

TEST(ModuleDocument, ANoOpApplyAnnouncesNothing) {
  ModuleDocument uut{openFixture("read/simple_binary_expr.fl")};
  QSignalSpy changed(&uut, &ModuleDocument::changed);

  EXPECT_FALSE(uut.apply(moveTo(kConstant, 2, 2)));

  EXPECT_EQ(changed.count(), 0);
}

TEST(ModuleDocument, RecordAnnouncesTheChange) {
  ModuleDocument uut{openFixture("read/simple_binary_expr.fl")};
  QSignalSpy changed(&uut, &ModuleDocument::changed);

  uut.record(moveTo(kConstant, 2, 2));

  EXPECT_EQ(changed.count(), 1);
  EXPECT_TRUE(uut.editor().canUndo());
}

TEST(ModuleDocument, UndoAndRedoAnnounceTheChange) {
  ModuleDocument uut{openFixture("read/simple_binary_expr.fl")};
  uut.apply(moveTo(kConstant, 7, 9));
  QSignalSpy changed(&uut, &ModuleDocument::changed);

  EXPECT_TRUE(uut.undo());
  EXPECT_EQ(changed.count(), 1);
  EXPECT_TRUE(uut.redo());
  EXPECT_EQ(changed.count(), 2);
}

TEST(ModuleDocument, UndoAndRedoWithNoHistoryAnnounceNothing) {
  ModuleDocument uut{openFixture("read/simple_binary_expr.fl")};
  QSignalSpy changed(&uut, &ModuleDocument::changed);

  EXPECT_FALSE(uut.undo());
  EXPECT_FALSE(uut.redo());

  EXPECT_EQ(changed.count(), 0);
}

TEST(ModuleDocument, CanUndoChangedFiresOnlyWhenItFlips) {
  ModuleDocument uut{openFixture("read/simple_binary_expr.fl")};
  QSignalSpy canUndo(&uut, &ModuleDocument::canUndoChanged);

  uut.apply(moveTo(kConstant, 7, 9));
  uut.apply(moveTo(kConstant, 8, 9));
  ASSERT_EQ(canUndo.count(), 1);
  EXPECT_TRUE(canUndo.at(0).at(0).toBool());

  uut.undo();
  uut.undo();
  ASSERT_EQ(canUndo.count(), 2);
  EXPECT_FALSE(canUndo.at(1).at(0).toBool());
}

TEST(ModuleDocument, CanRedoChangedFiresOnlyWhenItFlips) {
  ModuleDocument uut{openFixture("read/simple_binary_expr.fl")};
  uut.apply(moveTo(kConstant, 7, 9));
  QSignalSpy canRedo(&uut, &ModuleDocument::canRedoChanged);

  uut.undo();
  ASSERT_EQ(canRedo.count(), 1);
  EXPECT_TRUE(canRedo.at(0).at(0).toBool());

  uut.apply(moveTo(kConstant, 8, 9));
  ASSERT_EQ(canRedo.count(), 2);
  EXPECT_FALSE(canRedo.at(1).at(0).toBool());
}

TEST(ModuleDocument, TouchAnnouncesAChangeOutsideTheHistory) {
  ModuleDocument uut{openFixture("read/conditional_with_body.fl")};
  QSignalSpy changed(&uut, &ModuleDocument::changed);
  const fluir::FullID conditional{1, 2};

  uut.touch([&](fluir::editor::et::ParseTree& tree) {
    std::get<fluir::editor::et::Conditional>(*fluir::editor::nodeAt(tree, conditional)).annotation.shownBranch =
      fluir::editor::ELSE_BRANCH_ID;
  });

  EXPECT_EQ(changed.count(), 1);
  EXPECT_FALSE(uut.editor().canUndo());
  EXPECT_EQ(std::get<fluir::editor::et::Conditional>(*fluir::editor::nodeAt(uut.editor().tree(), conditional))
              .annotation.shownBranch,
            fluir::editor::ELSE_BRANCH_ID);
}

TEST(ModuleDocument, SaveWritesTheTreeToItsProgram) {
  const fs::path program = scratchCopy("read/simple_binary_expr.fl", "save");
  std::optional<fluir::editor::ModuleEditor> editor = fluir::editor::openModule(program);
  ASSERT_TRUE(editor.has_value());
  ModuleDocument uut{std::move(*editor)};
  uut.apply(moveTo(kConstant, 7, 9));

  EXPECT_TRUE(uut.save());

  const testutil::Loaded reloaded = testutil::loadFixture(program.string());
  ASSERT_TRUE(reloaded.result.tree.has_value());
  EXPECT_EQ(*reloaded.result.tree, uut.editor().tree());
}

TEST(ModuleDocument, SaveOfANewModuleNeedsAPath) {
  ModuleDocument uut{fluir::editor::newModule()};

  EXPECT_FALSE(uut.save());
}

TEST(ModuleDocument, SaveAsMovesTheProgramAndAnnouncesIt) {
  ModuleDocument uut{fluir::editor::newModule()};
  QSignalSpy programChanged(&uut, &ModuleDocument::programChanged);
  const fs::path out = fs::temp_directory_path() / "fluir_document_test_save_as.fl";
  fs::remove(out);

  EXPECT_TRUE(uut.saveAs(out));

  EXPECT_TRUE(fs::exists(out));
  EXPECT_EQ(uut.editor().program(), out);
  EXPECT_EQ(programChanged.count(), 1);
}

TEST(ModuleDocument, SaveAsToAnUnwritablePathKeepsTheProgram) {
  ModuleDocument uut{fluir::editor::newModule()};
  QSignalSpy programChanged(&uut, &ModuleDocument::programChanged);

  EXPECT_FALSE(uut.saveAs("/nonexistent-dir/out.fl"));

  EXPECT_FALSE(uut.editor().program().has_value());
  EXPECT_EQ(programChanged.count(), 0);
}
