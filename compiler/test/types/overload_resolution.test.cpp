#include <gtest/gtest.h>

#include "compiler/types/symbol_table.hpp"

using fluir::Operator;

class TestOverloadResolution : public ::testing::Test {
 public:
  void SetUp() override {
    A = uut.addType(fluir::types::Type{"A"});
    B = uut.addType(fluir::types::Type{"B"});
    C = uut.addType(fluir::types::Type{"C"});
    D = uut.addType(fluir::types::Type{"D"});
    E = uut.addType(fluir::types::Type{"E"});
    F = uut.addType(fluir::types::Type{"F"});
    G = uut.addType(fluir::types::Type{"G"});
    H = uut.addType(fluir::types::Type{"H"});

    uut.addOperator(fluir::types::OperatorDefinition{A, Operator::PLUS, A, A});   // A + A => A
    uut.addOperator(fluir::types::OperatorDefinition{A, Operator::MINUS, A, A});  // A - A => A
    uut.addOperator(fluir::types::OperatorDefinition{A, Operator::SLASH, B, C});  // A + B => C
    uut.addOperator(fluir::types::OperatorDefinition{A, Operator::STAR, B, C});   // A - B => C
    uut.addOperator(fluir::types::OperatorDefinition{A, Operator::PLUS, C, C});   // A + C => C
    uut.addOperator(fluir::types::OperatorDefinition{A, Operator::MINUS, C, C});  // A - C => C
    uut.addOperator(fluir::types::OperatorDefinition{D, Operator::PLUS, D, D});   // C + D => D
    uut.addOperator(fluir::types::OperatorDefinition{D, Operator::MINUS, D, D});  // C - D => D
    uut.addOperator(fluir::types::OperatorDefinition{C, Operator::PLUS, C, C});   // C + C => C
    uut.addOperator(fluir::types::OperatorDefinition{C, Operator::MINUS, C, C});  // C - C => C
    uut.addOperator(fluir::types::OperatorDefinition{Operator::MINUS, A, A});     // + A => A
    uut.addOperator(fluir::types::OperatorDefinition{Operator::PLUS, A, A});      // - A => A
    uut.addOperator(fluir::types::OperatorDefinition{E, Operator::PLUS, E, E});   // E + E => E
    uut.addOperator(fluir::types::OperatorDefinition{E, Operator::MINUS, E, E});  // E - E => E
    uut.addOperator(fluir::types::OperatorDefinition{E, Operator::STAR, E, E});   // E * E => E
    uut.addOperator(fluir::types::OperatorDefinition{E, Operator::SLASH, E, E});  // E / E => E
    uut.addOperator(fluir::types::OperatorDefinition{Operator::MINUS, E, E});     // - E => E
    uut.addOperator(fluir::types::OperatorDefinition{Operator::PLUS, E, E});      // + E => E
    uut.addOperator(fluir::types::OperatorDefinition{F, Operator::PLUS, F, G});   // F + F => G
    uut.addOperator(fluir::types::OperatorDefinition{F, Operator::MINUS, F, G});  // F - F => G
    uut.addOperator(fluir::types::OperatorDefinition{F, Operator::STAR, F, G});   // F * F => G
    uut.addOperator(fluir::types::OperatorDefinition{F, Operator::SLASH, F, G});  // F / F => G
    uut.addOperator(fluir::types::OperatorDefinition{Operator::PLUS, F, F});      // - F => G

    uut.addExplicitConversion(D, E);  // D (e)=> E
    uut.addImplicitConversion(A, B);  // A (i)=> B
    uut.addExplicitConversion(C, D);  // C (e)=> D
    uut.addImplicitConversion(E, F);  // E (i)=> F
    uut.addImplicitConversion(D, A);  // D (i)=> A
    uut.addImplicitConversion(G, F);  // G (i)=> F
    uut.addImplicitConversion(G, E);  // G (i)=> E
    uut.addImplicitConversion(D, C);  // D (i)=> C
  }

  fluir::types::SymbolTable uut;

  fluir::types::Type const* A;
  fluir::types::Type const* B;
  fluir::types::Type const* C;
  fluir::types::Type const* D;
  fluir::types::Type const* E;
  fluir::types::Type const* F;
  fluir::types::Type const* G;
  fluir::types::Type const* H;
};

TEST_F(TestOverloadResolution, ResolvesBinaryWithNoConversions) {
  const fluir::types::OperatorDefinition expected{A, Operator::PLUS, A, A};

  const auto actual = uut.selectOverload(A, Operator::PLUS, A);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, ResolvesUnaryWithNoConversions) {
  const fluir::types::OperatorDefinition expected{Operator::PLUS, A, A};

  const auto actual = uut.selectOverload(Operator::PLUS, A);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, ResolvesBinaryWithDifferentTypes) {
  const fluir::types::OperatorDefinition expected{A, Operator::SLASH, B, C};

  const auto actual = uut.selectOverload(A, Operator::SLASH, B);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, ResolvesBinaryWithRhsCast) {
  const fluir::types::OperatorDefinition expected{A, Operator::SLASH, B, C};

  const auto actual = uut.selectOverload(A, Operator::SLASH, A);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, ResolvesBinaryWithLhsCast) {
  const fluir::types::OperatorDefinition expected{F, Operator::PLUS, F, G};

  const auto actual = uut.selectOverload(E, Operator::PLUS, F);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, ResolvesUnaryWithSingleCast) {
  const fluir::types::OperatorDefinition expected{Operator::MINUS, E, E};

  const auto actual = uut.selectOverload(Operator::MINUS, G);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, ResolvesBinaryWithMultipleCastOptions) {
  const fluir::types::OperatorDefinition expected{E, Operator::MINUS, E, E};

  const auto actual = uut.selectOverload(E, Operator::MINUS, G);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, ResolvesUnaryWithConversion) {
  const fluir::types::OperatorDefinition expected{Operator::MINUS, E, E};

  const auto actual = uut.selectOverload(Operator::MINUS, G);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, FailsBinaryWithNoViableCandidates) {
  const auto actual = uut.selectOverload(H, Operator::PLUS, H);

  ASSERT_FALSE(actual);
}

TEST_F(TestOverloadResolution, FailsBinaryWithAmbiguousCall) {
  const auto actual = uut.selectOverload(H, Operator::PLUS, H);

  ASSERT_FALSE(actual);
}

TEST_F(TestOverloadResolution, FailsUnaryWithAmbiguousCall) {
  const auto actual = uut.selectOverload(Operator::PLUS, G);

  ASSERT_FALSE(actual);
}
