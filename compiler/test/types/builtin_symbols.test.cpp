#include "compiler/types/builtin_symbols.hpp"

#include <string>
#include <tuple>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include "compiler/types/symbol_table.hpp"
#include "compiler/types/type.hpp"

namespace ft = fluir::types;
using std::tuple;

class TestBuiltinTypes : public ::testing::TestWithParam<fluir::types::Type> {
 public:
  fluir::types::SymbolTable uut;

  void SetUp() override { fluir::types::instantiateBuiltinTypes(uut); }
};

TEST_P(TestBuiltinTypes, Test) {
  auto& expected = GetParam();

  auto actual = uut.getType(uut.getTypeID(expected.name()));

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

INSTANTIATE_TEST_SUITE_P(,
                         TestBuiltinTypes,
                         ::testing::Values(ft::Type{"F64"},
                                           ft::Type{"I8"},
                                           ft::Type{"I16"},
                                           ft::Type{"I32"},
                                           ft::Type{"I64"},
                                           ft::Type{"U8"},
                                           ft::Type{"U16"},
                                           ft::Type{"U32"},
                                           ft::Type{"U64"}));

class TestBuiltinOperators
  : public ::testing::TestWithParam<std::tuple<std::string, fluir::Operator, std::string, std::string>> {
 public:
  fluir::types::SymbolTable uut;

  void SetUp() override {
    fluir::types::instantiateBuiltinTypes(uut);
    fluir::types::instantiateBuiltinOperators(uut);
  }
};

TEST_P(TestBuiltinOperators, Test) {
  auto& [lhs, op, rhs, ret] = GetParam();

  auto expected = [&]() {
    if (rhs.empty()) {
      return fluir::types::OperatorDefinition{op, uut.getTypeID(lhs), uut.getTypeID(ret)};
    } else {
      return fluir::types::OperatorDefinition{uut.getTypeID(lhs), op, uut.getTypeID(rhs), uut.getTypeID(ret)};
    }
  }();

  auto actual = [&]() {
    if (rhs.empty()) {
      return uut.selectOverload(op, uut.getTypeID(lhs));
    } else {
      return uut.selectOverload(uut.getTypeID(lhs), op, uut.getTypeID(rhs));
    }
  }();

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

INSTANTIATE_TEST_SUITE_P(,
                         TestBuiltinOperators,
                         ::testing::Values(tuple{"F64", fluir::Operator::PLUS, "F64", "F64"},
                                           tuple{"F64", fluir::Operator::MINUS, "F64", "F64"},
                                           tuple{"F64", fluir::Operator::STAR, "F64", "F64"},
                                           tuple{"F64", fluir::Operator::SLASH, "F64", "F64"},
                                           tuple{"F64", fluir::Operator::PLUS, "", "F64"},
                                           tuple{"F64", fluir::Operator::MINUS, "", "F64"},
                                           tuple{"I8", fluir::Operator::PLUS, "I8", "I8"},
                                           tuple{"I8", fluir::Operator::MINUS, "I8", "I8"},
                                           tuple{"I8", fluir::Operator::STAR, "I8", "I8"},
                                           tuple{"I8", fluir::Operator::SLASH, "I8", "I8"},
                                           tuple{"I8", fluir::Operator::PLUS, "", "I8"},
                                           tuple{"I8", fluir::Operator::MINUS, "", "I8"},
                                           tuple{"I16", fluir::Operator::PLUS, "I16", "I16"},
                                           tuple{"I16", fluir::Operator::MINUS, "I16", "I16"},
                                           tuple{"I16", fluir::Operator::STAR, "I16", "I16"},
                                           tuple{"I16", fluir::Operator::SLASH, "I16", "I16"},
                                           tuple{"I16", fluir::Operator::PLUS, "", "I16"},
                                           tuple{"I16", fluir::Operator::MINUS, "", "I16"},
                                           tuple{"I32", fluir::Operator::PLUS, "I32", "I32"},
                                           tuple{"I32", fluir::Operator::MINUS, "I32", "I32"},
                                           tuple{"I32", fluir::Operator::STAR, "I32", "I32"},
                                           tuple{"I32", fluir::Operator::SLASH, "I32", "I32"},
                                           tuple{"I32", fluir::Operator::PLUS, "", "I32"},
                                           tuple{"I32", fluir::Operator::MINUS, "", "I32"},
                                           tuple{"I64", fluir::Operator::PLUS, "I64", "I64"},
                                           tuple{"I64", fluir::Operator::MINUS, "I64", "I64"},
                                           tuple{"I64", fluir::Operator::STAR, "I64", "I64"},
                                           tuple{"I64", fluir::Operator::SLASH, "I64", "I64"},
                                           tuple{"I64", fluir::Operator::PLUS, "", "I64"},
                                           tuple{"I64", fluir::Operator::MINUS, "", "I64"},
                                           tuple{"U8", fluir::Operator::PLUS, "U8", "U8"},
                                           tuple{"U8", fluir::Operator::MINUS, "U8", "U8"},
                                           tuple{"U8", fluir::Operator::STAR, "U8", "U8"},
                                           tuple{"U8", fluir::Operator::SLASH, "U8", "U8"},
                                           tuple{"U8", fluir::Operator::PLUS, "", "U8"},
                                           tuple{"U16", fluir::Operator::PLUS, "U16", "U16"},
                                           tuple{"U16", fluir::Operator::MINUS, "U16", "U16"},
                                           tuple{"U16", fluir::Operator::STAR, "U16", "U16"},
                                           tuple{"U16", fluir::Operator::SLASH, "U16", "U16"},
                                           tuple{"U16", fluir::Operator::PLUS, "", "U16"},
                                           tuple{"U32", fluir::Operator::PLUS, "U32", "U32"},
                                           tuple{"U32", fluir::Operator::MINUS, "U32", "U32"},
                                           tuple{"U32", fluir::Operator::STAR, "U32", "U32"},
                                           tuple{"U32", fluir::Operator::SLASH, "U32", "U32"},
                                           tuple{"U32", fluir::Operator::PLUS, "", "U32"},
                                           tuple{"U64", fluir::Operator::PLUS, "U64", "U64"},
                                           tuple{"U64", fluir::Operator::MINUS, "U64", "U64"},
                                           tuple{"U64", fluir::Operator::STAR, "U64", "U64"},
                                           tuple{"U64", fluir::Operator::SLASH, "U64", "U64"},
                                           tuple{"U64", fluir::Operator::PLUS, "", "U64"}));

class TestBuiltinCasts : public ::testing::TestWithParam<std::tuple<bool, std::string, std::string>> {
 public:
  fluir::types::SymbolTable uut;

  void SetUp() override {
    fluir::types::instantiateBuiltinTypes(uut);
    fluir::types::instantiateBuiltinCasts(uut);
  }
};

TEST_P(TestBuiltinCasts, Test) {
  auto& [isImplicit, sourceName, targetName] = GetParam();

  auto source = uut.getTypeID(sourceName);
  auto target = uut.getTypeID(targetName);

  if (isImplicit) {
    EXPECT_TRUE(uut.canExplicitlyConvert(source, target));
    EXPECT_TRUE(uut.canImplicitlyConvert(source, target));
  } else {
    EXPECT_TRUE(uut.canExplicitlyConvert(source, target));
    EXPECT_FALSE(uut.canImplicitlyConvert(source, target));
  }
}

INSTANTIATE_TEST_SUITE_P(,
                         TestBuiltinCasts,
                         ::testing::Values(tuple{true, "I8", "I16"},
                                           tuple{true, "I8", "I32"},
                                           tuple{true, "I8", "I64"},
                                           tuple{true, "I8", "F64"},
                                           tuple{true, "I16", "I32"},
                                           tuple{true, "I16", "I64"},
                                           tuple{true, "I16", "F64"},
                                           tuple{true, "I32", "I64"},
                                           tuple{true, "I32", "F64"},
                                           tuple{true, "I64", "F64"},
                                           tuple{true, "U8", "U16"},
                                           tuple{true, "U8", "U32"},
                                           tuple{true, "U8", "U64"},
                                           tuple{true, "U8", "F64"},
                                           tuple{true, "U16", "U32"},
                                           tuple{true, "U16", "U64"},
                                           tuple{true, "U16", "F64"},
                                           tuple{true, "U32", "U64"},
                                           tuple{true, "U32", "F64"},
                                           tuple{true, "U64", "F64"}));
