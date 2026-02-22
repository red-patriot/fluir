#include "compiler/types/function_type.hpp"

#include <gtest/gtest.h>

#include "compiler/types/symbol_table.hpp"
#include "compiler/types/typeid.hpp"

namespace ft = fluir::types;

TEST(TestFunctionType, NameIsAccessible) {
  ft::FunctionType func{"myFunc", {ft::ID_I32, ft::ID_F64}, ft::ID_I64};
  EXPECT_EQ(func.name(), "myFunc");
}

TEST(TestFunctionType, ParametersAreAccessible) {
  ft::FunctionType func{"myFunc", {ft::ID_I32, ft::ID_F64}, ft::ID_I64};
  EXPECT_EQ(func.parameters(), (std::vector<ft::TypeID>{ft::ID_I32, ft::ID_F64}));
}

TEST(TestFunctionType, ReturnTypeIsAccessible) {
  ft::FunctionType func{"myFunc", {ft::ID_I32, ft::ID_F64}, ft::ID_I64};
  ASSERT_TRUE(func.returnType().has_value());
  EXPECT_EQ(func.returnType().value(), ft::ID_I64);
}

TEST(TestFunctionType, NoParametersNoReturn) {
  ft::FunctionType func{"voidFunc", {}, std::nullopt};
  EXPECT_EQ(func.name(), "voidFunc");
  EXPECT_TRUE(func.parameters().empty());
  EXPECT_FALSE(func.returnType().has_value());
}

// --- SymbolTable::addFunction / getFunctionType tests ---

TEST(TestSymbolTable_Functions, GetFunctionTypeOnEmptyTableReturnsNullptr) {
  ft::SymbolTable table;
  EXPECT_EQ(table.getFunctionType("missing"), nullptr);
}

TEST(TestSymbolTable_Functions, AddThenGetFunctionByName) {
  ft::SymbolTable table;
  ft::FunctionType func{"add", {ft::ID_I32, ft::ID_I32}, ft::ID_I32};
  auto* added = table.addFunction(func);
  ASSERT_NE(added, nullptr);
  EXPECT_EQ(added->name(), "add");

  auto* found = table.getFunctionType("add");
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(found->name(), "add");
  EXPECT_EQ(found->parameters(), (std::vector<ft::TypeID>{ft::ID_I32, ft::ID_I32}));
  ASSERT_TRUE(found->returnType().has_value());
  EXPECT_EQ(found->returnType().value(), ft::ID_I32);
}

TEST(TestSymbolTable_Functions, DuplicateAddReturnsNullptr) {
  ft::SymbolTable table;
  ft::FunctionType func{"add", {ft::ID_I32}, ft::ID_I32};
  table.addFunction(func);
  EXPECT_EQ(table.addFunction(func), nullptr);
}

TEST(TestSymbolTable_Functions, DuplicateAddLeavesOriginalUnchanged) {
  ft::SymbolTable table;
  table.addFunction(ft::FunctionType{"add", {ft::ID_I32}, ft::ID_I32});
  table.addFunction(ft::FunctionType{"add", {ft::ID_F64}, ft::ID_F64});  // duplicate name

  auto* found = table.getFunctionType("add");
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(found->parameters(), (std::vector<ft::TypeID>{ft::ID_I32}));
}

TEST(TestSymbolTable_Functions, FunctionWithNoReturn) {
  ft::SymbolTable table;
  auto* added = table.addFunction(ft::FunctionType{"printVal", {ft::ID_I32}, std::nullopt});
  ASSERT_NE(added, nullptr);
  EXPECT_FALSE(added->returnType().has_value());
}

TEST(TestSymbolTable_Functions, FunctionWithNoParameters) {
  ft::SymbolTable table;
  auto* added = table.addFunction(ft::FunctionType{"getConst", {}, ft::ID_F64});
  ASSERT_NE(added, nullptr);
  EXPECT_TRUE(added->parameters().empty());
  ASSERT_TRUE(added->returnType().has_value());
  EXPECT_EQ(added->returnType().value(), ft::ID_F64);
}

TEST(TestSymbolTable_Functions, MultipleFunctionsStoredIndependently) {
  ft::SymbolTable table;
  table.addFunction(ft::FunctionType{"foo", {ft::ID_I32}, ft::ID_I32});
  table.addFunction(ft::FunctionType{"bar", {ft::ID_F64}, ft::ID_F64});

  ASSERT_NE(table.getFunctionType("foo"), nullptr);
  ASSERT_NE(table.getFunctionType("bar"), nullptr);
  EXPECT_EQ(table.getFunctionType("baz"), nullptr);
  EXPECT_EQ(table.getFunctionType("foo")->parameters(), (std::vector<ft::TypeID>{ft::ID_I32}));
  EXPECT_EQ(table.getFunctionType("bar")->parameters(), (std::vector<ft::TypeID>{ft::ID_F64}));
}
