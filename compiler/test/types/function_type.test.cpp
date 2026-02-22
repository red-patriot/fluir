#include "compiler/types/function_type.hpp"

#include <gtest/gtest.h>

#include "compiler/types/symbol_table.hpp"
#include "compiler/types/typeid.hpp"

namespace ft = fluir::types;

TEST(TestFunctionType, ParametersAreAccessible) {
  ft::FunctionType func{{ft::ID_I32, ft::ID_F64}, ft::ID_I64};
  EXPECT_EQ(func.parameters(), (std::vector<ft::TypeID>{ft::ID_I32, ft::ID_F64}));
}

TEST(TestFunctionType, ReturnTypeIsAccessible) {
  ft::FunctionType func{{ft::ID_I32, ft::ID_F64}, ft::ID_I64};
  ASSERT_TRUE(func.returnType().has_value());
  EXPECT_EQ(func.returnType().value(), ft::ID_I64);
}

TEST(TestFunctionType, NoParametersNoReturn) {
  ft::FunctionType func{{}, std::nullopt};
  EXPECT_TRUE(func.parameters().empty());
  EXPECT_FALSE(func.returnType().has_value());
}

TEST(TestFunctionType, EqualityHoldsForSameSignature) {
  ft::FunctionType a{{ft::ID_I32, ft::ID_F64}, ft::ID_I64};
  ft::FunctionType b{{ft::ID_I32, ft::ID_F64}, ft::ID_I64};
  EXPECT_EQ(a, b);
}

TEST(TestFunctionType, EqualityFailsForDifferentParams) {
  ft::FunctionType a{{ft::ID_I32}, ft::ID_I64};
  ft::FunctionType b{{ft::ID_F64}, ft::ID_I64};
  EXPECT_NE(a, b);
}

TEST(TestFunctionType, EqualityFailsForDifferentReturn) {
  ft::FunctionType a{{ft::ID_I32}, ft::ID_I64};
  ft::FunctionType b{{ft::ID_I32}, ft::ID_F64};
  EXPECT_NE(a, b);
}

// --- SymbolTable::addFunction / getFunctionType tests ---

TEST(TestSymbolTable, GetFunctionTypeOnEmptyTableReturnsNullptr) {
  ft::SymbolTable table;
  EXPECT_EQ(table.getFunctionType("missing"), nullptr);
}

TEST(TestSymbolTable, AddThenGetFunctionByName) {
  ft::SymbolTable table;
  auto* added = table.addFunction("add", ft::FunctionType{{ft::ID_I32, ft::ID_I32}, ft::ID_I32});
  ASSERT_NE(added, nullptr);

  auto* found = table.getFunctionType("add");
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(found->parameters(), (std::vector<ft::TypeID>{ft::ID_I32, ft::ID_I32}));
  ASSERT_TRUE(found->returnType().has_value());
  EXPECT_EQ(found->returnType().value(), ft::ID_I32);
}

TEST(TestSymbolTable, DuplicateAddReturnsNullptr) {
  ft::SymbolTable table;
  table.addFunction("add", ft::FunctionType{{ft::ID_I32}, ft::ID_I32});
  EXPECT_EQ(table.addFunction("add", ft::FunctionType{{ft::ID_I32}, ft::ID_I32}), nullptr);
}

TEST(TestSymbolTable, DuplicateAddLeavesOriginalUnchanged) {
  ft::SymbolTable table;
  table.addFunction("add", ft::FunctionType{{ft::ID_I32}, ft::ID_I32});
  table.addFunction("add", ft::FunctionType{{ft::ID_F64}, ft::ID_F64});  // duplicate name

  auto* found = table.getFunctionType("add");
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(found->parameters(), (std::vector<ft::TypeID>{ft::ID_I32}));
}

TEST(TestSymbolTable, FunctionWithNoReturn) {
  ft::SymbolTable table;
  auto* added = table.addFunction("printVal", ft::FunctionType{{ft::ID_I32}, std::nullopt});
  ASSERT_NE(added, nullptr);
  EXPECT_FALSE(added->returnType().has_value());
}

TEST(TestSymbolTable, FunctionWithNoParameters) {
  ft::SymbolTable table;
  auto* added = table.addFunction("getConst", ft::FunctionType{{}, ft::ID_F64});
  ASSERT_NE(added, nullptr);
  EXPECT_TRUE(added->parameters().empty());
  ASSERT_TRUE(added->returnType().has_value());
  EXPECT_EQ(added->returnType().value(), ft::ID_F64);
}

TEST(TestSymbolTable, MultipleFunctionsStoredIndependently) {
  ft::SymbolTable table;
  table.addFunction("foo", ft::FunctionType{{ft::ID_I32}, ft::ID_I32});
  table.addFunction("bar", ft::FunctionType{{ft::ID_F64}, ft::ID_F64});

  ASSERT_NE(table.getFunctionType("foo"), nullptr);
  ASSERT_NE(table.getFunctionType("bar"), nullptr);
  EXPECT_EQ(table.getFunctionType("baz"), nullptr);
  EXPECT_EQ(table.getFunctionType("foo")->parameters(), (std::vector<ft::TypeID>{ft::ID_I32}));
  EXPECT_EQ(table.getFunctionType("bar")->parameters(), (std::vector<ft::TypeID>{ft::ID_F64}));
}

// --- getFunctionTypeID tests ---

TEST(TestSymbolTable, GetFunctionTypeIDReturnsInvalidForUnknownFunction) {
  ft::SymbolTable table;
  EXPECT_EQ(table.getFunctionTypeID("missing"), ft::TypeID::ID_INVALID);
}

TEST(TestSymbolTable, GetFunctionTypeIDReturnsValidIDAfterAdd) {
  ft::SymbolTable table;
  table.addFunction("foo", ft::FunctionType{{ft::ID_I32}, ft::ID_I64});
  EXPECT_NE(table.getFunctionTypeID("foo"), ft::TypeID::ID_INVALID);
}

TEST(TestSymbolTable, SameSignatureGetsSameTypeID) {
  ft::SymbolTable table;
  table.addFunction("foo", ft::FunctionType{{ft::ID_I32, ft::ID_I32}, ft::ID_I64});
  table.addFunction("bar", ft::FunctionType{{ft::ID_I32, ft::ID_I32}, ft::ID_I64});
  EXPECT_EQ(table.getFunctionTypeID("foo"), table.getFunctionTypeID("bar"));
}

TEST(TestSymbolTable, DifferentSignaturesGetDifferentTypeIDs) {
  ft::SymbolTable table;
  table.addFunction("foo", ft::FunctionType{{ft::ID_I32}, ft::ID_I64});
  table.addFunction("bar", ft::FunctionType{{ft::ID_F64}, ft::ID_I64});
  EXPECT_NE(table.getFunctionTypeID("foo"), table.getFunctionTypeID("bar"));
}

TEST(TestSymbolTable, FunctionTypeIDDoesNotCollideWithPrimitiveTypeIDs) {
  ft::SymbolTable table;
  // Add the nine builtin primitive types (as builtin_symbols.cpp would do)
  table.addType(ft::Type{"F64"});
  table.addType(ft::Type{"I8"});
  table.addType(ft::Type{"I16"});
  table.addType(ft::Type{"I32"});
  table.addType(ft::Type{"I64"});
  table.addType(ft::Type{"U8"});
  table.addType(ft::Type{"U16"});
  table.addType(ft::Type{"U32"});
  table.addType(ft::Type{"U64"});

  table.addFunction("foo", ft::FunctionType{{ft::ID_I32}, ft::ID_I64});
  const auto funcID = table.getFunctionTypeID("foo");

  // Must not collide with any builtin TypeID
  EXPECT_NE(funcID, ft::TypeID::ID_INVALID);
  EXPECT_NE(funcID, ft::TypeID::ID_F64);
  EXPECT_NE(funcID, ft::TypeID::ID_I8);
  EXPECT_NE(funcID, ft::TypeID::ID_I16);
  EXPECT_NE(funcID, ft::TypeID::ID_I32);
  EXPECT_NE(funcID, ft::TypeID::ID_I64);
  EXPECT_NE(funcID, ft::TypeID::ID_U8);
  EXPECT_NE(funcID, ft::TypeID::ID_U16);
  EXPECT_NE(funcID, ft::TypeID::ID_U32);
  EXPECT_NE(funcID, ft::TypeID::ID_U64);
}
