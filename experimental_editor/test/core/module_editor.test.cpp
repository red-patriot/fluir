#include "editor/core/module_editor.hpp"

#include <memory>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "editor/core/tree.hpp"
#include "editor/transaction/delete.hpp"
#include "editor/transaction/move.hpp"
#include "fixture_loader.hpp"

// The editor's contract is its history: what apply and record keep, what undo
// and redo reach, and what load forgets. MoveTransaction is the probe: it is
// self-inverting and reports a no-op move as false.

namespace {

  using fluir::editor::DeleteTransaction;
  using fluir::editor::ModuleEditor;
  using fluir::editor::MoveTransaction;
  using testutil::loadFixture;

  // simple_binary_expr.fl: function 1 "foo" holding binary 1, constants 2 and
  // 3, and conduits 4 (2 -> 1 slot 0) and 5 (3 -> 1 slot 1).
  const fluir::FullID kConstant{1, 2};
  constexpr int kConstantX = 2; /**< the constant's position in the fixture */
  constexpr int kConstantY = 2;

  // Mirrors the .cpp's file-local cap; tests may not read that constant.
  constexpr int kHistoryLimit = 100;

  std::unique_ptr<MoveTransaction> moveTo(int x, int y) { return std::make_unique<MoveTransaction>(kConstant, x, y); }

  ModuleEditor loadedEditor() {
    const testutil::Loaded loaded = loadFixture("read/simple_binary_expr.fl");
    EXPECT_TRUE(loaded.result.tree.has_value());
    ModuleEditor editor;
    editor.load(loaded.result.tree.value_or(fluir::editor::et::ParseTree{}));
    return editor;
  }

}  // namespace

TEST(ModuleEditor, ApplyOfANoOpEditIsNotRecorded) {
  ModuleEditor uut = loadedEditor();
  const fluir::editor::et::ParseTree before = uut.tree();

  EXPECT_FALSE(uut.apply(moveTo(kConstantX, kConstantY)));
  EXPECT_FALSE(uut.canUndo());
  EXPECT_EQ(uut.tree(), before);
}

TEST(ModuleEditor, ApplyRecordsTheEdit) {
  ModuleEditor uut = loadedEditor();
  const fluir::editor::et::ParseTree before = uut.tree();

  EXPECT_TRUE(uut.apply(moveTo(7, 9)));
  EXPECT_TRUE(uut.canUndo());
  EXPECT_FALSE(uut.canRedo());
  EXPECT_NE(uut.tree(), before);
}

TEST(ModuleEditor, UndoRestoresTheTreeAndOffersRedo) {
  ModuleEditor uut = loadedEditor();
  const fluir::editor::et::ParseTree before = uut.tree();
  ASSERT_TRUE(uut.apply(moveTo(7, 9)));

  EXPECT_TRUE(uut.undo());
  EXPECT_EQ(uut.tree(), before);
  EXPECT_FALSE(uut.canUndo());
  EXPECT_TRUE(uut.canRedo());
}

TEST(ModuleEditor, RedoReachesTheSameStateAsTheFirstApply) {
  ModuleEditor uut = loadedEditor();
  ASSERT_TRUE(uut.apply(moveTo(7, 9)));
  const fluir::editor::et::ParseTree afterFirst = uut.tree();
  ASSERT_TRUE(uut.undo());

  EXPECT_TRUE(uut.redo());
  EXPECT_EQ(uut.tree(), afterFirst);
  EXPECT_TRUE(uut.canUndo());
  EXPECT_FALSE(uut.canRedo());
}

TEST(ModuleEditor, UndoAndRedoWithNoHistoryReturnFalseAndChangeNothing) {
  ModuleEditor uut = loadedEditor();
  const fluir::editor::et::ParseTree before = uut.tree();

  EXPECT_FALSE(uut.undo());
  EXPECT_FALSE(uut.redo());
  EXPECT_EQ(uut.tree(), before);
}

TEST(ModuleEditor, ApplyingAfterAnUndoForgetsTheRedoPath) {
  ModuleEditor uut = loadedEditor();
  ASSERT_TRUE(uut.apply(moveTo(7, 9)));
  ASSERT_TRUE(uut.undo());
  ASSERT_TRUE(uut.canRedo());

  ASSERT_TRUE(uut.apply(moveTo(20, 30)));

  EXPECT_FALSE(uut.canRedo());
  EXPECT_FALSE(uut.redo());
}

// A live gesture has already executed its edit; record only keeps it.
TEST(ModuleEditor, RecordKeepsAnAlreadyExecutedEditWithoutReapplyingIt) {
  ModuleEditor uut = loadedEditor();
  const fluir::editor::et::ParseTree before = uut.tree();
  auto edit = moveTo(7, 9);
  ASSERT_TRUE(edit->execute(uut.tree()));
  const fluir::editor::et::ParseTree moved = uut.tree();

  uut.record(std::move(edit));

  EXPECT_EQ(uut.tree(), moved);
  EXPECT_TRUE(uut.undo());
  EXPECT_EQ(uut.tree(), before);
}

TEST(ModuleEditor, RecordForgetsTheRedoPath) {
  ModuleEditor uut = loadedEditor();
  ASSERT_TRUE(uut.apply(moveTo(7, 9)));
  ASSERT_TRUE(uut.undo());
  auto edit = moveTo(20, 30);
  ASSERT_TRUE(edit->execute(uut.tree()));

  uut.record(std::move(edit));

  EXPECT_FALSE(uut.canRedo());
}

// Undoing past the cap must stop at the state the dropped edit left behind --
// the first moved-to position -- never at the fixture's original position.
TEST(ModuleEditor, HistoryIsBoundedAndDropsTheOldestEdit) {
  ModuleEditor uut = loadedEditor();
  const fluir::editor::et::ParseTree original = uut.tree();

  ASSERT_TRUE(uut.apply(moveTo(100, 100)));
  const fluir::editor::et::ParseTree afterFirst = uut.tree();
  for (int i = 1; i <= kHistoryLimit; ++i) {
    ASSERT_TRUE(uut.apply(moveTo(100 + i, 100 + i)));
  }

  while (uut.undo()) { }

  EXPECT_EQ(uut.tree(), afterFirst);
  EXPECT_NE(uut.tree(), original);
}

TEST(ModuleEditor, LoadReplacesTheTreeAndForgetsTheHistory) {
  ModuleEditor uut = loadedEditor();
  ASSERT_TRUE(uut.apply(moveTo(7, 9)));
  ASSERT_TRUE(uut.apply(moveTo(20, 30)));
  ASSERT_TRUE(uut.undo());

  uut.load(fluir::editor::et::ParseTree{});

  EXPECT_FALSE(uut.canUndo());
  EXPECT_FALSE(uut.canRedo());
  EXPECT_TRUE(uut.tree().declarations.empty());
}

TEST(ModuleEditor, UndoOfADeleteRestoresTheNodeAndItsConduits) {
  ModuleEditor uut = loadedEditor();
  const fluir::editor::et::ParseTree before = uut.tree();

  ASSERT_TRUE(uut.apply(std::make_unique<DeleteTransaction>(kConstant)));
  const auto& body = std::get<fluir::editor::et::FunctionDecl>(uut.tree().declarations.at(1)).body;
  ASSERT_EQ(body.nodes.count(2), 0u);
  ASSERT_EQ(body.conduits.count(4), 0u);

  EXPECT_TRUE(uut.undo());
  EXPECT_EQ(uut.tree(), before);
}

namespace {

  // Top-level declarations 3 and 7; function 7 takes param 20, returns 25, holds node 10 and conduit 15.
  fluir::editor::et::ParseTree makeIdTree() {
    fluir::editor::et::FunctionDecl fn;
    fn.id = 7;
    fn.name = "f";
    fn.input = fluir::editor::et::FunctionDecl::InputBlock{
      .parameters = {{.id = 20, .index = 0, .name = "x", .typeName = "i32"}}};
    fn.output = fluir::editor::et::FunctionDecl::OutputBlock{
      .ret = fluir::editor::et::FunctionDecl::Return{.id = 25, .typeName = "i32"}};
    fn.body.nodes.emplace(10, fluir::editor::et::Comment{.id = 10, .location = {}, .text = ""});
    fn.body.conduits.emplace(15, fluir::editor::et::Conduit{.id = 15});
    fluir::editor::et::ParseTree tree;
    tree.declarations.emplace(3, fluir::editor::et::Comment{.id = 3, .location = {}, .text = ""});
    tree.declarations.emplace(7, fn);
    return tree;
  }

  fluir::editor::et::Block& idTreeBody(ModuleEditor& editor) {
    return std::get<fluir::editor::et::FunctionDecl>(editor.tree().declarations.at(7)).body;
  }

}  // namespace

TEST(ModuleEditor, GenerateIDOnAnEmptyTreeIsValid) {
  const ModuleEditor uut;

  EXPECT_NE(uut.generateID({}), fluir::INVALID_ID);
}

TEST(ModuleEditor, GenerateIDTopLevelExceedsEveryDeclaration) {
  ModuleEditor uut;
  uut.load(makeIdTree());

  EXPECT_GT(uut.generateID({}), 7u);
}

TEST(ModuleEditor, GenerateIDInABodyExceedsEveryNodeConduitParamAndReturn) {
  ModuleEditor uut;
  uut.load(makeIdTree());
  EXPECT_GT(uut.generateID({7}), 25u) << "return is the largest";

  idTreeBody(uut).conduits.emplace(30, fluir::editor::et::Conduit{.id = 30});
  EXPECT_GT(uut.generateID({7}), 30u) << "conduit is the largest";

  idTreeBody(uut).nodes.emplace(35, fluir::editor::et::Comment{.id = 35, .location = {}, .text = ""});
  EXPECT_GT(uut.generateID({7}), 35u) << "node is the largest";

  std::get<fluir::editor::et::FunctionDecl>(uut.tree().declarations.at(7))
    .input->parameters.push_back({.id = 40, .index = 1, .name = "y", .typeName = "i32"});
  EXPECT_GT(uut.generateID({7}), 40u) << "param is the largest";
}

TEST(ModuleEditor, GenerateIDForAnUnknownBodyIsInvalid) {
  ModuleEditor uut;
  uut.load(makeIdTree());

  EXPECT_EQ(uut.generateID({99}), fluir::INVALID_ID);
  EXPECT_EQ(uut.generateID({3}), fluir::INVALID_ID) << "a comment has no body";
}
