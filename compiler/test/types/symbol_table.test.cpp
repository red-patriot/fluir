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

TEST(TestSymbolTable, StoresConversions) {
  fluir::types::SymbolTable uut;

  const auto A = uut.addType(fluir::types::Type{"A"});
  const auto B = uut.addType(fluir::types::Type{"B"});
  const auto C = uut.addType(fluir::types::Type{"C"});
  uut.addImplicitConversion(A, B);
  uut.addExplicitConversion(A, C);

  EXPECT_TRUE(uut.canConvert(A, B));
  EXPECT_TRUE(uut.canCast(A, B));

  EXPECT_TRUE(uut.canConvert(A, C));
  EXPECT_FALSE(uut.canCast(A, C));

  EXPECT_FALSE(uut.canCast(B, C));
  EXPECT_FALSE(uut.canCast(C, A));
  EXPECT_FALSE(uut.canCast(B, A));
}

TEST(TestSymbolTable, SelfConversionsAreIgnored) {
  fluir::types::SymbolTable uut;

  uut.addType(fluir::types::Type{"A"});
  uut.addImplicitConversion(uut.getType("A"), uut.getType(("A")));

  EXPECT_FALSE(uut.canConvert(uut.getType("A"), uut.getType("A")));
  EXPECT_FALSE(uut.canCast(uut.getType("A"), uut.getType("A")));
}
