#include "compiler/types/symbol_table.hpp"

#include <algorithm>

#include <gtest/gtest.h>

#include "compiler/types/operator_def.hpp"

TEST(TestSymbolTable, AddBinaryOperator) {
  fluir::types::SymbolTable uut;
  const auto lhs = &fluir::types::BOOL();
  const auto rhs = &fluir::types::EMPTY();
  const auto ret = &fluir::types::I8();

  fluir::types::OperatorDefinition op{lhs, fluir::Operator::STAR, rhs, ret};

  uut.addOperator(op);
  auto overloads = uut.getOperatorOverloads(fluir::Operator::STAR);

  EXPECT_EQ(1, overloads.size());
}

TEST(TestSymbolTable, AddUnaryOperator) {
  fluir::types::SymbolTable uut;
  const auto lhs = &fluir::types::EMPTY();
  const auto ret = &fluir::types::BOOL();

  fluir::types::OperatorDefinition op{fluir::Operator::STAR, lhs, ret};

  uut.addOperator(op);
  auto overloads = uut.getOperatorOverloads(fluir::Operator::STAR);

  EXPECT_EQ(1, overloads.size());
}

TEST(TestSymbolTable, UnaryAndBinaryDiffer) {
  fluir::types::SymbolTable uut;
  const auto lhs = &fluir::types::EMPTY();
  const auto rhs = &fluir::types::BOOL();
  const auto ret = &fluir::types::U64();

  fluir::types::OperatorDefinition binary{lhs, fluir::Operator::MINUS, rhs, ret};
  fluir::types::OperatorDefinition unary{fluir::Operator::MINUS, ret, lhs};

  uut.addOperator(binary);
  uut.addOperator(unary);
  auto overloads = uut.getOperatorOverloads(fluir::Operator::MINUS);

  EXPECT_EQ(2, overloads.size());
}

TEST(TestSymbolTable, CannotAddDuplicateOperatorOverloads) {
  fluir::types::SymbolTable uut;
  const auto lhs = &fluir::types::EMPTY();
  const auto rhs = &fluir::types::BOOL();
  const auto ret = &fluir::types::U64();

  fluir::types::OperatorDefinition op{lhs, fluir::Operator::PLUS, rhs, ret};

  uut.addOperator(op);
  uut.addOperator(op);
  auto overloads = uut.getOperatorOverloads(fluir::Operator::PLUS);

  EXPECT_EQ(1, overloads.size());
}

TEST(TestSymbolTable, ErrorIfAddingOverloadsOnlyDistinguishedByReturnType) {
  fluir::types::SymbolTable uut;
  const auto lhs = &fluir::types::EMPTY();
  const auto rhs = &fluir::types::BOOL();
  const auto ret1 = &fluir::types::U64();
  const auto ret2 = &fluir::types::I64();

  EXPECT_TRUE(uut.addOperator(fluir::types::OperatorDefinition{lhs, fluir::Operator::PLUS, rhs, ret1}));
  EXPECT_FALSE(uut.addOperator(fluir::types::OperatorDefinition{lhs, fluir::Operator::PLUS, rhs, ret2}));
}

TEST(TestSymbolTable, StoresConversions) {
  fluir::types::SymbolTable uut;

  const auto A = &fluir::types::EMPTY();
  const auto B = &fluir::types::BOOL();
  const auto C = &fluir::types::U64();
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

  const auto A = &fluir::types::EMPTY();
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

  const auto A = &fluir::types::EMPTY();
  const auto B = &fluir::types::BOOL();
  const auto C = &fluir::types::U64();

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

  const auto A = &fluir::types::I8();

  uut.pushScope();
  uut.addLocalVariable(1, A);

  auto actual = uut.getLocalVariableType(3);

  EXPECT_EQ(nullptr, actual);
}

TEST(TestSymbolTable, PoppingScopeRemovesLocalVariables) {
  fluir::types::SymbolTable uut;

  const auto A = &fluir::types::EMPTY();
  const auto B = &fluir::types::BOOL();
  const auto C = &fluir::types::U64();

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
  EXPECT_EQ(nullptr, actual2);
  EXPECT_EQ(C, actual3);
}

TEST(TestSymbolTable, OnlyCurrentLocalScopeIsSearchedForVariables) {
  fluir::types::SymbolTable uut;

  const auto A = &fluir::types::EMPTY();
  const auto B = &fluir::types::BOOL();

  uut.pushScope();
  uut.addLocalVariable(1, A);
  uut.pushScope();

  uut.addLocalVariable(2, B);

  auto actual1 = uut.getLocalVariableType(1);
  auto actual2 = uut.getLocalVariableType(2);

  EXPECT_EQ(nullptr, actual1);
  EXPECT_EQ(B, actual2);
}
