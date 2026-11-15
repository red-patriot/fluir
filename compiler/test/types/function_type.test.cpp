#include "compiler/types/function_type.hpp"

#include <gtest/gtest.h>

#include "compiler/types/symbol_table.hpp"

namespace ft = fluir::types;

TEST(TestFunctionType, ParametersAreAccessible) {
  ft::FunctionType func{{&ft::I32(), &ft::F64()}, &ft::I64()};
  EXPECT_EQ(func.parameters(), (std::vector<fluir::types::TypePtr>{&ft::I32(), &ft::F64()}));
}

TEST(TestFunctionType, ReturnTypeIsAccessible) {
  ft::FunctionType func{{&ft::I32(), &ft::F64()}, &ft::I64()};
  EXPECT_EQ(func.returnType(), &ft::I64());
}

TEST(TestFunctionType, NoParametersNoReturn) {
  ft::FunctionType func{{}, &ft::EMPTY()};
  EXPECT_TRUE(func.parameters().empty());
  EXPECT_EQ(func.returnType(), &ft::EMPTY());
}

TEST(TestFunctionType, EqualityHoldsForSameSignature) {
  ft::FunctionType a{{&ft::I32(), &ft::F64()}, &ft::I64()};
  ft::FunctionType b{{&ft::I32(), &ft::F64()}, &ft::I64()};
  EXPECT_EQ(a, b);
}

TEST(TestFunctionType, EqualityFailsForDifferentParams) {
  ft::FunctionType a{{&ft::I32()}, &ft::I64()};
  ft::FunctionType b{{&ft::F64()}, &ft::I64()};
  EXPECT_NE(a, b);
}

TEST(TestFunctionType, EqualityFailsForDifferentReturn) {
  ft::FunctionType a{{&ft::I32()}, &ft::I64()};
  ft::FunctionType b{{&ft::I32()}, &ft::F64()};
  EXPECT_NE(a, b);
}

// --- SymbolTable::addFunction / getFunctionType tests ---

TEST(TestSymbolTable, GetFunctionTypeOnEmptyTableReturnsNullptr) {
  ft::SymbolTable table;
  EXPECT_EQ(table.getFunctionType("missing"), nullptr);
}

TEST(TestSymbolTable, AddThenGetFunctionByName) {
  ft::SymbolTable table;
  const auto addedID = table.addFunction("add", ft::FunctionType{{&ft::I32(), &ft::I32()}, &ft::I32()});
  ASSERT_TRUE(addedID);

  auto* found = table.getFunctionType("add");
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(found->parameters(), (std::vector<fluir::types::TypePtr>{&ft::I32(), &ft::I32()}));
  EXPECT_EQ(found->returnType(), &ft::I32());
}

TEST(TestSymbolTable, DuplicateAddReturnsExisting) {
  ft::SymbolTable table;
  table.addFunction("add", ft::FunctionType{{&ft::I32()}, &ft::I32()});
  EXPECT_NE(table.addFunction("add", ft::FunctionType{{&ft::I32()}, &ft::I32()}), nullptr);
}

TEST(TestSymbolTable, DuplicateAddLeavesOriginalUnchanged) {
  ft::SymbolTable table;
  table.addFunction("add", ft::FunctionType{{&ft::I32()}, &ft::I32()});
  table.addFunction("add", ft::FunctionType{{&ft::F64()}, &ft::F64()});  // duplicate name

  auto* found = table.getFunctionType("add");
  ASSERT_TRUE(found);
  EXPECT_EQ(found->parameters(), (std::vector<fluir::types::TypePtr>{&ft::I32()}));
}

TEST(TestSymbolTable, FunctionWithNoReturn) {
  ft::SymbolTable table;
  const auto addedID = table.addFunction("printVal", ft::FunctionType{{&ft::I32()}, &ft::EMPTY()});
  ASSERT_TRUE(addedID);
  auto* found = table.getFunctionType("printVal");
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(found->returnType(), &ft::EMPTY());
}

TEST(TestSymbolTable, FunctionWithNoParameters) {
  ft::SymbolTable table;
  const auto addedID = table.addFunction("getConst", ft::FunctionType{{}, &ft::F64()});
  ASSERT_TRUE(addedID);
  auto* found = table.getFunctionType("getConst");
  ASSERT_TRUE(found);
  EXPECT_TRUE(found->parameters().empty());
  EXPECT_EQ(found->returnType(), &ft::F64());
}

TEST(TestSymbolTable, MultipleFunctionsStoredIndependently) {
  ft::SymbolTable table;
  table.addFunction("foo", ft::FunctionType{{&ft::I32()}, &ft::I32()});
  table.addFunction("bar", ft::FunctionType{{&ft::F64()}, &ft::F64()});

  ASSERT_NE(table.getFunctionType("foo"), nullptr);
  ASSERT_NE(table.getFunctionType("bar"), nullptr);
  EXPECT_EQ(table.getFunctionType("baz"), nullptr);
  EXPECT_EQ(table.getFunctionType("foo")->parameters(), (std::vector<fluir::types::TypePtr>{&ft::I32()}));
  EXPECT_EQ(table.getFunctionType("bar")->parameters(), (std::vector<fluir::types::TypePtr>{&ft::F64()}));
}

// --- getFunctionTypeID tests ---

TEST(TestSymbolTable, GetFunctionTypeIDReturnsInvalidForUnknownFunction) {
  ft::SymbolTable table;
  EXPECT_EQ(table.getFunctionType("missing"), nullptr);
}

TEST(TestSymbolTable, GetFunctionTypeReturnsValidIDAfterAdd) {
  ft::SymbolTable table;
  table.addFunction("foo", ft::FunctionType{{&ft::I32()}, &ft::I64()});
  EXPECT_NE(table.getFunctionType("foo"), nullptr);
}

TEST(TestSymbolTable, SameSignatureGetsSameTypeID) {
  ft::SymbolTable table;
  table.addFunction("foo", ft::FunctionType{{&ft::I32(), &ft::I32()}, &ft::I64()});
  table.addFunction("bar", ft::FunctionType{{&ft::I32(), &ft::I32()}, &ft::I64()});
  EXPECT_EQ(table.getFunctionType("foo"), table.getFunctionType("bar"));
}

TEST(TestSymbolTable, DifferentSignaturesGetDifferentTypeIDs) {
  ft::SymbolTable table;
  table.addFunction("foo", ft::FunctionType{{&ft::I32()}, &ft::I64()});
  table.addFunction("bar", ft::FunctionType{{&ft::F64()}, &ft::I64()});
  EXPECT_NE(table.getFunctionType("foo"), table.getFunctionType("bar"));
}
