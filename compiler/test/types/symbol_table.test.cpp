#include "compiler/types/symbol_table.hpp"

#include <algorithm>

#include <gtest/gtest.h>

#include "compiler/types/operator_def.hpp"

TEST(TestSymbolTable, AddGetType) {
  fluir::types::Type expected{"SimpleType1"};

  fluir::types::SymbolTable uut;

  uut.addType(expected);

  auto actual = uut.getType("SimpleType1");

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);

  auto notFound = uut.getType("UnknownType");
  EXPECT_FALSE(notFound);
}

TEST(TestSymbolTable, AddBinaryOperator) {
  fluir::types::SymbolTable uut;
  uut.addType(fluir::types::Type{"Lhs"});
  uut.addType(fluir::types::Type{"Rhs"});
  uut.addType(fluir::types::Type{"Ret"});

  fluir::types::OperatorDefinition op{
    uut.getType("Lhs"), fluir::Operator::STAR, uut.getType("Rhs"), uut.getType("Ret")};

  uut.addOperator(op);
  auto overloads = uut.getOperatorOverloads(fluir::Operator::STAR);

  EXPECT_EQ(1, overloads.size());
}

TEST(TestSymbolTable, AddUnaryOperator) {
  fluir::types::SymbolTable uut;
  uut.addType(fluir::types::Type{"Lhs"});
  uut.addType(fluir::types::Type{"Ret"});

  fluir::types::OperatorDefinition op{fluir::Operator::STAR, uut.getType("Lhs"), uut.getType("Ret")};

  uut.addOperator(op);
  auto overloads = uut.getOperatorOverloads(fluir::Operator::STAR);

  EXPECT_EQ(1, overloads.size());
}

TEST(TestSymbolTable, UnaryAndBinaryDiffer) {
  fluir::types::SymbolTable uut;
  uut.addType(fluir::types::Type{"Lhs"});
  uut.addType(fluir::types::Type{"Rhs"});
  uut.addType(fluir::types::Type{"Ret"});

  fluir::types::OperatorDefinition binary{
    uut.getType("Lhs"), fluir::Operator::MINUS, uut.getType("Rhs"), uut.getType("Ret")};
  fluir::types::OperatorDefinition unary{fluir::Operator::MINUS, uut.getType("Ret"), uut.getType("Lhs")};

  uut.addOperator(binary);
  uut.addOperator(unary);
  auto overloads = uut.getOperatorOverloads(fluir::Operator::MINUS);

  EXPECT_EQ(2, overloads.size());
}

TEST(TestSymbolTable, CannotAddDuplicateOperatorOverloads) {
  fluir::types::SymbolTable uut;
  uut.addType(fluir::types::Type{"Lhs"});
  uut.addType(fluir::types::Type{"Rhs"});
  uut.addType(fluir::types::Type{"Ret"});

  fluir::types::OperatorDefinition op{
    uut.getType("Lhs"), fluir::Operator::PLUS, uut.getType("Rhs"), uut.getType("Ret")};

  uut.addOperator(op);
  uut.addOperator(op);
  auto overloads = uut.getOperatorOverloads(fluir::Operator::PLUS);

  EXPECT_EQ(1, overloads.size());
}

TEST(TestSymbolTable, ErrorIfAddingOverloadsOnlyDistinguishedByReturnType) {
  fluir::types::SymbolTable uut;
  auto lhs = uut.addType(fluir::types::Type{"Lhs"});
  auto rhs = uut.addType(fluir::types::Type{"Rhs"});
  auto ret1 = uut.addType(fluir::types::Type{"Ret1"});
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

  uut.addType(fluir::types::Type{"A"});
  uut.addImplicitConversion(uut.getType("A"), uut.getType(("A")));

  EXPECT_FALSE(uut.canExplicitlyConvert(uut.getType("A"), uut.getType("A")));
  EXPECT_FALSE(uut.canImplicitlyConvert(uut.getType("A"), uut.getType("A")));
}
