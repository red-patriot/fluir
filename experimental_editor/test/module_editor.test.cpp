#include "editor/core/module_editor.hpp"

#include <memory>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/actors/scene.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/scene_to_tree.hpp"
#include "editor/transaction/delete.hpp"
#include "editor/transaction/move.hpp"
#include "fixture_loader.hpp"

// The editor's contract is its history: what apply records, what undo and redo
// reach, and what reset forgets -- each asserted through sceneToParseTree, a
// complete description of the scene. MoveTransaction is the probe: it is
// self-inverting and reports a no-op move as false.

namespace {

  using fluir::editor::DeleteTransaction;
  using fluir::editor::EditorContext;
  using fluir::editor::ModuleEditor;
  using fluir::editor::MoveTransaction;
  using fluir::editor::sceneToParseTree;
  using testutil::Loaded;
  using testutil::loadFixture;

  const EditorContext kCtx;
  const fluir::pt::Header kHeader{.version = {0, 1, 3}};

  // simple_binary_expr.fl: function 1 "foo" holding binary 1, constants 2 and
  // 3, and conduits 4 (2 -> 1 slot 0) and 5 (3 -> 1 slot 1).
  const fluir::FullID kConstant{1, 2};
  constexpr int kConstantX = 2; /**< the constant's position in the fixture */
  constexpr int kConstantY = 2;

  // Mirrors the .cpp's file-local cap; tests may not read that constant.
  constexpr int kHistoryLimit = 100;

  std::unique_ptr<MoveTransaction> moveTo(int x, int y) { return std::make_unique<MoveTransaction>(kConstant, x, y); }

  // The fixture, built into an editor's own scene; the Loaded must outlive it.
  void buildFixture(ModuleEditor& editor, const Loaded& loaded) {
    ASSERT_TRUE(loaded.result.tree.has_value());
    editor.scene().build(kCtx, *loaded.result.tree);
  }

}  // namespace

TEST(ModuleEditor, ApplyOfANoOpEditIsNotRecorded) {
  const Loaded loaded = loadFixture("read/simple_binary_expr.fl");
  ModuleEditor uut;
  buildFixture(uut, loaded);
  const fluir::pt::ParseTree before = sceneToParseTree(uut.scene(), kHeader);

  EXPECT_FALSE(uut.apply(moveTo(kConstantX, kConstantY)));
  EXPECT_FALSE(uut.canUndo());
  EXPECT_EQ(sceneToParseTree(uut.scene(), kHeader), before);
}

TEST(ModuleEditor, ApplyRecordsTheEdit) {
  const Loaded loaded = loadFixture("read/simple_binary_expr.fl");
  ModuleEditor uut;
  buildFixture(uut, loaded);
  const fluir::pt::ParseTree before = sceneToParseTree(uut.scene(), kHeader);

  EXPECT_TRUE(uut.apply(moveTo(7, 9)));
  EXPECT_TRUE(uut.canUndo());
  EXPECT_FALSE(uut.canRedo());
  EXPECT_NE(sceneToParseTree(uut.scene(), kHeader), before);
}

TEST(ModuleEditor, UndoRestoresTheSceneAndOffersRedo) {
  const Loaded loaded = loadFixture("read/simple_binary_expr.fl");
  ModuleEditor uut;
  buildFixture(uut, loaded);
  const fluir::pt::ParseTree before = sceneToParseTree(uut.scene(), kHeader);
  ASSERT_TRUE(uut.apply(moveTo(7, 9)));

  EXPECT_TRUE(uut.undo());
  EXPECT_EQ(sceneToParseTree(uut.scene(), kHeader), before);
  EXPECT_FALSE(uut.canUndo());
  EXPECT_TRUE(uut.canRedo());
}

TEST(ModuleEditor, RedoReachesTheSameStateAsTheFirstApply) {
  const Loaded loaded = loadFixture("read/simple_binary_expr.fl");
  ModuleEditor uut;
  buildFixture(uut, loaded);
  ASSERT_TRUE(uut.apply(moveTo(7, 9)));
  const fluir::pt::ParseTree afterFirst = sceneToParseTree(uut.scene(), kHeader);
  ASSERT_TRUE(uut.undo());

  EXPECT_TRUE(uut.redo());
  EXPECT_EQ(sceneToParseTree(uut.scene(), kHeader), afterFirst);
  EXPECT_TRUE(uut.canUndo());
  EXPECT_FALSE(uut.canRedo());
}

TEST(ModuleEditor, UndoWithNoHistoryReturnsFalseAndChangesNothing) {
  const Loaded loaded = loadFixture("read/simple_binary_expr.fl");
  ModuleEditor uut;
  buildFixture(uut, loaded);
  const fluir::pt::ParseTree before = sceneToParseTree(uut.scene(), kHeader);

  EXPECT_FALSE(uut.undo());
  EXPECT_EQ(sceneToParseTree(uut.scene(), kHeader), before);
}

TEST(ModuleEditor, RedoWithNoHistoryReturnsFalseAndChangesNothing) {
  const Loaded loaded = loadFixture("read/simple_binary_expr.fl");
  ModuleEditor uut;
  buildFixture(uut, loaded);
  const fluir::pt::ParseTree before = sceneToParseTree(uut.scene(), kHeader);

  EXPECT_FALSE(uut.redo());
  EXPECT_EQ(sceneToParseTree(uut.scene(), kHeader), before);
}

TEST(ModuleEditor, ApplyingAfterAnUndoForgetsTheRedoPath) {
  const Loaded loaded = loadFixture("read/simple_binary_expr.fl");
  ModuleEditor uut;
  buildFixture(uut, loaded);
  ASSERT_TRUE(uut.apply(moveTo(7, 9)));
  ASSERT_TRUE(uut.undo());
  ASSERT_TRUE(uut.canRedo());

  ASSERT_TRUE(uut.apply(moveTo(20, 30)));

  EXPECT_FALSE(uut.canRedo());
  EXPECT_FALSE(uut.redo());
}

// Undoing past the cap must stop at the state the dropped edit left behind --
// the first moved-to position -- never at the fixture's original position.
TEST(ModuleEditor, HistoryIsBoundedAndDropsTheOldestEdit) {
  const Loaded loaded = loadFixture("read/simple_binary_expr.fl");
  ModuleEditor uut;
  buildFixture(uut, loaded);
  const fluir::pt::ParseTree original = sceneToParseTree(uut.scene(), kHeader);

  ASSERT_TRUE(uut.apply(moveTo(100, 100)));
  const fluir::pt::ParseTree afterFirst = sceneToParseTree(uut.scene(), kHeader);
  for (int i = 1; i <= kHistoryLimit; ++i) {
    ASSERT_TRUE(uut.apply(moveTo(100 + i, 100 + i)));
  }

  while (uut.undo()) { }

  EXPECT_EQ(sceneToParseTree(uut.scene(), kHeader), afterFirst);
  EXPECT_NE(sceneToParseTree(uut.scene(), kHeader), original);
}

TEST(ModuleEditor, ResetForgetsTheHistory) {
  const Loaded loaded = loadFixture("read/simple_binary_expr.fl");
  ModuleEditor uut;
  buildFixture(uut, loaded);
  ASSERT_TRUE(uut.apply(moveTo(7, 9)));
  ASSERT_TRUE(uut.apply(moveTo(20, 30)));
  ASSERT_TRUE(uut.undo());

  uut.reset();

  EXPECT_FALSE(uut.canUndo());
  EXPECT_FALSE(uut.canRedo());
  EXPECT_FALSE(uut.undo());
  EXPECT_FALSE(uut.redo());
  EXPECT_TRUE(sceneToParseTree(uut.scene(), kHeader).declarations.empty());
}

// The editor must keep the transaction -- and so the DetachedActors it holds --
// alive for as long as the undo is reachable.
TEST(ModuleEditor, UndoOfADeleteRestoresTheNodeAndItsConduits) {
  const Loaded loaded = loadFixture("read/simple_binary_expr.fl");
  ModuleEditor uut;
  buildFixture(uut, loaded);
  const fluir::pt::ParseTree before = sceneToParseTree(uut.scene(), kHeader);

  ASSERT_TRUE(uut.apply(std::make_unique<DeleteTransaction>(kConstant)));
  const fluir::pt::ParseTree afterDelete = sceneToParseTree(uut.scene(), kHeader);
  const auto& body = std::get<fluir::pt::FunctionDecl>(afterDelete.declarations.at(1)).body;
  ASSERT_EQ(body.nodes.count(2), 0u);
  ASSERT_EQ(body.conduits.count(4), 0u);

  EXPECT_TRUE(uut.undo());
  EXPECT_EQ(sceneToParseTree(uut.scene(), kHeader), before);
}
