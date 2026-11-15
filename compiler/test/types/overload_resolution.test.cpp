#include <gtest/gtest.h>

#include "compiler/types/symbol_table.hpp"
#include "compiler/types/type_arena.hpp"

using fluir::Operator;
using fluir::types::OperatorDefinition;
using fluir::types::Type;
using fluir::types::TypePtr;
namespace ft = fluir::types;

class TestOverloadResolution : public ::testing::Test {
 public:
  void SetUp() override {
    A = arena.addProduct({&ft::BOOL()});
    B = arena.addProduct({&ft::I8()});
    C = arena.addProduct({&ft::I16()});
    D = arena.addProduct({&ft::I32()});
    E = arena.addProduct({&ft::I64()});
    F = arena.addProduct({&ft::U8()});
    G = arena.addProduct({&ft::U16()});
    H = arena.addProduct({&ft::U32()});

    uut.addOperator(OperatorDefinition{A, Operator::PLUS, A, A});   // A + A => A
    uut.addOperator(OperatorDefinition{A, Operator::MINUS, A, A});  // A - A => A
    uut.addOperator(OperatorDefinition{Operator::PLUS, A, A});      // + A   => A
    uut.addOperator(OperatorDefinition{Operator::MINUS, A, A});     // - A   => A
    uut.addOperator(OperatorDefinition{D, Operator::PLUS, D, D});   // D + D => D
    uut.addOperator(OperatorDefinition{D, Operator::MINUS, D, D});  // D - D => D
    uut.addOperator(OperatorDefinition{Operator::PLUS, C, C});      // + C   => C
    uut.addOperator(OperatorDefinition{Operator::MINUS, D, D});     // - D   => D
    uut.addOperator(OperatorDefinition{E, Operator::STAR, F, F});   // E * F => F
    uut.addOperator(OperatorDefinition{E, Operator::STAR, D, E});   // E * D => E
    uut.addOperator(OperatorDefinition{A, Operator::SLASH, D, A});  // A / D => A
    uut.addOperator(OperatorDefinition{A, Operator::SLASH, F, F});  // A / F => F
    uut.addOperator(OperatorDefinition{Operator::PLUS, F, F});      // + F   => F
    uut.addOperator(OperatorDefinition{Operator::MINUS, F, F});     // - F   => F
    uut.addOperator(OperatorDefinition{A, Operator::SLASH, H, C});  // A / H => C
    uut.addOperator(OperatorDefinition{Operator::MINUS, H, H});     // - H   => H

    uut.addImplicitConversion(B, A);  // B (i)-> A
    uut.addImplicitConversion(E, D);  // E (i)-> D
    uut.addImplicitConversion(E, F);  // E (i)-> F
    uut.addExplicitConversion(A, D);  // A (e)-> D
    uut.addExplicitConversion(G, C);  // G (e)-> C
    uut.addImplicitConversion(G, D);  // G (i)-> D
    uut.addImplicitConversion(H, F);  // H (i)-> F
    uut.addExplicitConversion(G, F);  // G (e)-> F
  }

  fluir::types::TypeArena arena;
  fluir::types::SymbolTable uut;

  TypePtr A{nullptr};
  TypePtr B{nullptr};
  TypePtr C{nullptr};
  TypePtr D{nullptr};
  TypePtr E{nullptr};
  TypePtr F{nullptr};
  TypePtr G{nullptr};
  TypePtr H{nullptr};
};

TEST_F(TestOverloadResolution, ResolvesBinaryNoCast) {
  const OperatorDefinition expected{A, Operator::PLUS, A, A};

  const auto actual = uut.selectOverload(A, Operator::PLUS, A);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, ResolvesUnaryNoCast) {
  const OperatorDefinition expected{Operator::MINUS, A, A};

  const auto actual = uut.selectOverload(Operator::MINUS, A);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, ResolvesUnary1Cast) {
  const OperatorDefinition expected{Operator::PLUS, A, A};

  const auto actual = uut.selectOverload(Operator::PLUS, B);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, ResolvesBinaryFirstCast) {
  const OperatorDefinition expected{A, Operator::PLUS, A, A};

  const auto actual = uut.selectOverload(B, Operator::PLUS, A);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, ResolvesBinarySecondCast) {
  const OperatorDefinition expected{A, Operator::MINUS, A, A};

  const auto actual = uut.selectOverload(A, Operator::MINUS, B);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, FailsIfMultipleCastsRequired) {
  const auto actual = uut.selectOverload(B, Operator::PLUS, B);

  EXPECT_FALSE(actual);
}

TEST_F(TestOverloadResolution, FailsBinaryIfNoViableCandidates) {
  const auto actual = uut.selectOverload(C, Operator::PLUS, C);

  EXPECT_FALSE(actual);
}

TEST_F(TestOverloadResolution, FailsUnaryIfNoViableCandidates) {
  const auto actual = uut.selectOverload(Operator::MINUS, C);

  EXPECT_FALSE(actual);
}

TEST_F(TestOverloadResolution, FailsBinaryIfCallIsAmbiguous) {
  const auto actual = uut.selectOverload(E, Operator::STAR, E);

  EXPECT_FALSE(actual);
}

TEST_F(TestOverloadResolution, FailsUnaryIfCallIsAmbiguous) {
  const auto actual = uut.selectOverload(Operator::MINUS, E);

  EXPECT_FALSE(actual);
}

TEST_F(TestOverloadResolution, FailsBinaryIfCallRequiresExplicitConversions) {
  const auto actual = uut.selectOverload(E, Operator::STAR, A);

  EXPECT_FALSE(actual);
}

TEST_F(TestOverloadResolution, FailsUnaryIfCallRequiresExplicitConversions) {
  const auto actual = uut.selectOverload(Operator::PLUS, G);

  EXPECT_FALSE(actual);
}

TEST_F(TestOverloadResolution, ResolvingExplicitAndImplicitBinaryAcceptsOnlyImplicit) {
  const OperatorDefinition expected{A, Operator::SLASH, D, A};

  const auto actual = uut.selectOverload(A, Operator::SLASH, G);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, ResolvingExplicitAndImplicitUnaryAcceptsOnlyImplicit) {
  const OperatorDefinition expected{Operator::MINUS, D, D};

  const auto actual = uut.selectOverload(Operator::MINUS, G);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, ResolvingCastAndNoCastBinaryAcceptsNoCast) {
  const OperatorDefinition expected{A, Operator::SLASH, H, C};

  const auto actual = uut.selectOverload(A, Operator::SLASH, H);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

TEST_F(TestOverloadResolution, ResolvingCastAndNoCastUnaryAcceptsNoCast) {
  const OperatorDefinition expected{Operator::MINUS, H, H};

  const auto actual = uut.selectOverload(Operator::MINUS, H);

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}
