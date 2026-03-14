#include <gtest/gtest.h>

#include "fluir/testing/test_files_dir.hpp"
#include "lsp/database/in_memory_db.hpp"

namespace fs = std::filesystem;

namespace {
  fs::path findTestFile(const fs::path& relative) { return fluir::test::getTestProgram(relative); }
}  // namespace

class SymbolTreeTest : public ::testing::Test {
 protected:
  fluir::CompilerOptions opts_{.developerOptions = {.suppressVersionErrors = true}};
  fluir::lsp::InMemoryDB db_{opts_};
};

// --- return_widening.fl ---

TEST_F(SymbolTreeTest, ReturnWideningDeclarationSymbol) {
  auto file = findTestFile("type_check/return_widening.fl");
  db_.setFileContents(file, fluir::test::readContents(file));

  auto sym = db_.symbolAt(file, {1});
  ASSERT_TRUE(sym.has_value());
  EXPECT_EQ(sym->name, "func main");
  EXPECT_EQ(sym->outType, "I32");
  EXPECT_EQ(sym->inType, std::nullopt);
}

TEST_F(SymbolTreeTest, ReturnWideningConstantSymbol) {
  auto file = findTestFile("type_check/return_widening.fl");
  db_.setFileContents(file, fluir::test::readContents(file));

  auto sym = db_.symbolAt(file, {1, 3});
  ASSERT_TRUE(sym.has_value());
  EXPECT_EQ(sym->name, "I8 Constant");
  EXPECT_EQ(sym->outType, "I8");
  EXPECT_EQ(sym->inType, std::nullopt);
}

// --- recursive.fl ---

TEST_F(SymbolTreeTest, RecursiveDeclarationSymbol) {
  auto file = findTestFile("type_check/recursive.fl");
  db_.setFileContents(file, fluir::test::readContents(file));

  auto sym = db_.symbolAt(file, {1});
  ASSERT_TRUE(sym.has_value());
  EXPECT_EQ(sym->name, "func self_recurse");
  ASSERT_TRUE(sym->inType.has_value());
  EXPECT_EQ(*sym->inType, std::vector<std::string>{"I32"});
  EXPECT_EQ(sym->outType, "I32");
}

TEST_F(SymbolTreeTest, RecursiveCallSymbol) {
  auto file = findTestFile("type_check/recursive.fl");
  db_.setFileContents(file, fluir::test::readContents(file));

  auto sym = db_.symbolAt(file, {1, 3});
  ASSERT_TRUE(sym.has_value());
  EXPECT_EQ(sym->name, "self_recurse");
  ASSERT_TRUE(sym->inType.has_value());
  EXPECT_EQ(*sym->inType, std::vector<std::string>{"I32"});
  EXPECT_EQ(sym->outType, "I32");
}

// --- func_call_simple.fl ---

TEST_F(SymbolTreeTest, FuncCallSimpleHasBothDeclarations) {
  auto file = findTestFile("type_check/func_call_simple.fl");
  db_.setFileContents(file, fluir::test::readContents(file));

  auto main_sym = db_.symbolAt(file, {1, 1});
  auto add_sym = db_.symbolAt(file, {2, 2});
  ASSERT_TRUE(main_sym.has_value());
  ASSERT_TRUE(add_sym.has_value());
}

TEST_F(SymbolTreeTest, FuncCallSimpleAddDeclSymbol) {
  auto file = findTestFile("type_check/func_call_simple.fl");
  db_.setFileContents(file, fluir::test::readContents(file));

  auto sym = db_.symbolAt(file, {2, 2});
  ASSERT_TRUE(sym.has_value());
  EXPECT_EQ(sym->name, "func add");
  ASSERT_TRUE(sym->inType.has_value());
  EXPECT_EQ(*sym->inType, (std::vector<std::string>{"I32", "I32"}));
  EXPECT_EQ(sym->outType, "I32");
}

TEST_F(SymbolTreeTest, FuncCallSimpleBinaryOpSymbol) {
  auto file = findTestFile("type_check/func_call_simple.fl");
  db_.setFileContents(file, fluir::test::readContents(file));

  auto sym = db_.symbolAt(file, {2, 4});
  ASSERT_TRUE(sym.has_value());
  EXPECT_EQ(sym->name, "binary +");
}

// --- forward_reference.fl ---

TEST_F(SymbolTreeTest, ForwardReferenceCallSymbol) {
  auto file = findTestFile("type_check/forward_reference.fl");
  db_.setFileContents(file, fluir::test::readContents(file));

  auto sym = db_.symbolAt(file, {1, 3});
  ASSERT_TRUE(sym.has_value());
  EXPECT_EQ(sym->name, "add");
  ASSERT_TRUE(sym->inType.has_value());
  EXPECT_EQ(*sym->inType, (std::vector<std::string>{"I32", "I32"}));
  EXPECT_EQ(sym->outType, "I32");
}

// --- Edge cases ---

TEST_F(SymbolTreeTest, SymbolAtInvalidIdReturnsNullopt) {
  auto file = findTestFile("type_check/return_widening.fl");
  db_.setFileContents(file, fluir::test::readContents(file));

  auto sym = db_.symbolAt(file, {999, 999});
  EXPECT_FALSE(sym.has_value());
}

TEST_F(SymbolTreeTest, SymbolAtEmptyTargetReturnsNullopt) {
  auto file = findTestFile("type_check/return_widening.fl");
  db_.setFileContents(file, fluir::test::readContents(file));

  auto sym = db_.symbolAt(file, {});
  EXPECT_FALSE(sym.has_value());
}
