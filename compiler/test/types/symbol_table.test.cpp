#include "compiler/types/symbol_table.hpp"

#include <algorithm>

#include <gtest/gtest.h>

#include "compiler/types/operator_def.hpp"

TEST(TestSymbolTable, AddGetTypeByID) {
  fluir::types::Type expected{"SimpleType1"};

  fluir::types::SymbolTable uut;

  const auto id = uut.addType(expected);

  const auto actual = uut.getType(id);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);

  const auto notFound = uut.getType(fluir::types::TypeID::ID_INVALID);
  EXPECT_FALSE(notFound);
}

TEST(TestSymbolTable, AddGetTypeByName) {
  fluir::types::Type expected{"SimpleType1"};

  fluir::types::SymbolTable uut;

  uut.addType(expected);
  const auto id = uut.getTypeID(expected.name());

  const auto actual = uut.getType(id);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);

  auto unknownID = uut.getTypeID("UnknownType");
  auto notFound = uut.getType(unknownID);
  EXPECT_FALSE(notFound);
}

TEST(TestSymbolTable, AddBinaryOperator) {
  fluir::types::SymbolTable uut;
  const auto lhs = uut.addType(fluir::types::Type{"Lhs"});
  const auto rhs = uut.addType(fluir::types::Type{"Rhs"});
  const auto ret = uut.addType(fluir::types::Type{"Ret"});

  fluir::types::OperatorDefinition op{lhs, fluir::Operator::STAR, rhs, ret};

  uut.addOperator(op);
  auto overloads = uut.getOperatorOverloads(fluir::Operator::STAR);

  EXPECT_EQ(1, overloads.size());
}

TEST(TestSymbolTable, AddUnaryOperator) {
  fluir::types::SymbolTable uut;
  const auto lhs = uut.addType(fluir::types::Type{"Lhs"});
  const auto ret = uut.addType(fluir::types::Type{"Ret"});

  fluir::types::OperatorDefinition op{fluir::Operator::STAR, lhs, ret};

  uut.addOperator(op);
  auto overloads = uut.getOperatorOverloads(fluir::Operator::STAR);

  EXPECT_EQ(1, overloads.size());
}

TEST(TestSymbolTable, UnaryAndBinaryDiffer) {
  fluir::types::SymbolTable uut;
  const auto lhs = uut.addType(fluir::types::Type{"Lhs"});
  const auto rhs = uut.addType(fluir::types::Type{"Rhs"});
  const auto ret = uut.addType(fluir::types::Type{"Ret"});

  fluir::types::OperatorDefinition binary{lhs, fluir::Operator::MINUS, rhs, ret};
  fluir::types::OperatorDefinition unary{fluir::Operator::MINUS, ret, lhs};

  uut.addOperator(binary);
  uut.addOperator(unary);
  auto overloads = uut.getOperatorOverloads(fluir::Operator::MINUS);

  EXPECT_EQ(2, overloads.size());
}

TEST(TestSymbolTable, CannotAddDuplicateOperatorOverloads) {
  fluir::types::SymbolTable uut;
  const auto lhs = uut.addType(fluir::types::Type{"Lhs"});
  const auto rhs = uut.addType(fluir::types::Type{"Rhs"});
  const auto ret = uut.addType(fluir::types::Type{"Ret"});

  fluir::types::OperatorDefinition op{lhs, fluir::Operator::PLUS, rhs, ret};

  uut.addOperator(op);
  uut.addOperator(op);
  auto overloads = uut.getOperatorOverloads(fluir::Operator::PLUS);

  EXPECT_EQ(1, overloads.size());
}

TEST(TestSymbolTable, ErrorIfAddingOverloadsOnlyDistinguishedByReturnType) {
  fluir::types::SymbolTable uut;
  auto lhs = uut.addType(fluir::types::Type{"Lhs"});
  auto rhs = uut.addType(fluir::types::Type{"Rhs"});
  auto ret1 = uut.addType(fluir::types::Type{"Ret"});
  auto ret2 = uut.addType(fluir::types::Type{"Ret2"});

  EXPECT_TRUE(uut.addOperator(fluir::types::OperatorDefinition{lhs, fluir::Operator::PLUS, rhs, ret1}));
  EXPECT_FALSE(uut.addOperator(fluir::types::OperatorDefinition{lhs, fluir::Operator::PLUS, rhs, ret2}));
}

TEST(TestSymbolTable, StoresConversions) {
  fluir::types::SymbolTable uut;

  const auto A = uut.addType(fluir::types::Type{"A"});
  const auto B = uut.addType(fluir::types::Type{"B"});
  const auto C = uut.addType(fluir::types::Type{"C"});
  uut.addImplicitConversion(A, B);
  uut.addExplicitConversion(A, C);

  EXPECT_TRUE(uut.canExplicitlyConvert(A, B));
  EXPECT_TRUE(uut.canImplicitlyConvert(A, B));

  EXPECT_TRUE(uut.canExplicitlyConvert(A, C));
  EXPECT_FALSE(uut.canImplicitlyConvert(A, C));

  EXPECT_FALSE(uut.canImplicitlyConvert(B, C));
  EXPECT_FALSE(uut.canImplicitlyConvert(C, A));
  EXPECT_FALSE(uut.canImplicitlyConvert(B, A));
}

TEST(TestSymbolTable, SelfConversionsAreIgnored) {
  fluir::types::SymbolTable uut;

  const auto A = uut.addType(fluir::types::Type{"A"});
  uut.addImplicitConversion(A, A);

  EXPECT_FALSE(uut.canExplicitlyConvert(A, A));
  EXPECT_FALSE(uut.canImplicitlyConvert(A, A));
}

TEST(TestSymbolTable, CanPushAndPopScopes) {
  fluir::types::SymbolTable uut;

  EXPECT_NO_THROW(uut.pushScope());
  EXPECT_NO_THROW(uut.pushScope());
  EXPECT_NO_THROW(uut.popScope());
  EXPECT_NO_THROW(uut.popScope());
  EXPECT_NO_THROW(uut.popScope());
}

TEST(TestSymbolTable, CanAddAndAccessLocalVariables) {
  fluir::types::SymbolTable uut;

  const auto A = uut.addType(fluir::types::Type{"A"});
  const auto B = uut.addType(fluir::types::Type{"B"});
  const auto C = uut.addType(fluir::types::Type{"C"});

  uut.pushScope();
  uut.addLocalVariable(1, A);
  uut.addLocalVariable(2, B);
  uut.addLocalVariable(3, C);
  uut.addLocalVariable(4, C);

  auto actual1 = uut.getLocalVariableType(1);
  auto actual2 = uut.getLocalVariableType(2);
  auto actual3 = uut.getLocalVariableType(3);
  auto actual4 = uut.getLocalVariableType(4);

  EXPECT_EQ(A, actual1);
  EXPECT_EQ(B, actual2);
  EXPECT_EQ(C, actual3);
  EXPECT_EQ(C, actual4);
}

TEST(TestSymbolTable, ReadingNonexistingVariableReturnsInvalid) {
  fluir::types::SymbolTable uut;

  const auto A = uut.addType(fluir::types::Type{"A"});

  uut.pushScope();
  uut.addLocalVariable(1, A);

  auto actual = uut.getLocalVariableType(3);

  EXPECT_EQ(fluir::types::TypeID::ID_INVALID, actual);
}

TEST(TestSymbolTable, PoppingScopeRemovesLocalVariables) {
  fluir::types::SymbolTable uut;

  const auto A = uut.addType(fluir::types::Type{"A"});
  const auto B = uut.addType(fluir::types::Type{"B"});
  const auto C = uut.addType(fluir::types::Type{"C"});

  uut.pushScope();
  uut.pushScope();
  uut.addLocalVariable(1, A);
  uut.addLocalVariable(2, B);
  auto actual1 = uut.getLocalVariableType(1);

  uut.popScope();
  uut.addLocalVariable(3, C);

  auto actual2 = uut.getLocalVariableType(1);
  auto actual3 = uut.getLocalVariableType(3);

  uut.popScope();

  EXPECT_EQ(A, actual1);
  EXPECT_EQ(fluir::types::TypeID::ID_INVALID, actual2);
  EXPECT_EQ(C, actual3);
}

TEST(TestSymbolTable, OnlyCurrentLocalScopeIsSearchedForVariables) {
  fluir::types::SymbolTable uut;

  const auto A = uut.addType(fluir::types::Type{"A"});
  const auto B = uut.addType(fluir::types::Type{"B"});

  uut.pushScope();
  uut.addLocalVariable(1, A);
  uut.pushScope();

  uut.addLocalVariable(2, B);

  auto actual1 = uut.getLocalVariableType(1);
  auto actual2 = uut.getLocalVariableType(2);

  EXPECT_EQ(fluir::types::TypeID::ID_INVALID, actual1);
  EXPECT_EQ(B, actual2);
}
