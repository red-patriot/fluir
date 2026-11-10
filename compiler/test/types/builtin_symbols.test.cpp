#include "compiler/types/builtin_symbols.hpp"

#include <string>
#include <tuple>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include "compiler/types/symbol_table.hpp"
#include "compiler/types/type.hpp"

namespace ft = fluir::types;
using std::tuple;

class TestBuiltinOperators
  : public ::testing::TestWithParam<std::tuple<ft::Type const*, fluir::Operator, ft::Type const*, ft::Type const*>> {
 public:
  fluir::types::SymbolTable uut;

  void SetUp() override { fluir::types::instantiateBuiltinOperators(uut); }
};

TEST_P(TestBuiltinOperators, Test) {
  auto& [lhs, op, rhs, ret] = GetParam();

  auto expected = [&]() {
    if (!rhs) {
      return fluir::types::OperatorDefinition{op, lhs, ret};
    } else {
      return fluir::types::OperatorDefinition{lhs, op, rhs, ret};
    }
  }();

  auto actual = [&]() {
    if (!rhs) {
      return uut.selectOverload(op, lhs);
    } else {
      return uut.selectOverload(lhs, op, rhs);
    }
  }();

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, *actual);
}

INSTANTIATE_TEST_SUITE_P(,
                         TestBuiltinOperators,
                         ::testing::Values(tuple{&ft::F64(), fluir::Operator::PLUS, &ft::F64(), &ft::F64()},
                                           tuple{&ft::F64(), fluir::Operator::MINUS, &ft::F64(), &ft::F64()},
                                           tuple{&ft::F64(), fluir::Operator::STAR, &ft::F64(), &ft::F64()},
                                           tuple{&ft::F64(), fluir::Operator::SLASH, &ft::F64(), &ft::F64()},
                                           tuple{&ft::F64(), fluir::Operator::PLUS, nullptr, &ft::F64()},
                                           tuple{&ft::F64(), fluir::Operator::MINUS, nullptr, &ft::F64()},
                                           tuple{&ft::F64(), fluir::Operator::PLUS_PLUS, nullptr, &ft::F64()},
                                           tuple{&ft::F64(), fluir::Operator::MINUS_MINUS, nullptr, &ft::F64()},
                                           tuple{&ft::I8(), fluir::Operator::PLUS, &ft::I8(), &ft::I8()},
                                           tuple{&ft::I8(), fluir::Operator::MINUS, &ft::I8(), &ft::I8()},
                                           tuple{&ft::I8(), fluir::Operator::STAR, &ft::I8(), &ft::I8()},
                                           tuple{&ft::I8(), fluir::Operator::SLASH, &ft::I8(), &ft::I8()},
                                           tuple{&ft::I8(), fluir::Operator::PLUS, nullptr, &ft::I8()},
                                           tuple{&ft::I8(), fluir::Operator::MINUS, nullptr, &ft::I8()},
                                           tuple{&ft::I8(), fluir::Operator::PLUS_PLUS, nullptr, &ft::I8()},
                                           tuple{&ft::I8(), fluir::Operator::MINUS_MINUS, nullptr, &ft::I8()},
                                           tuple{&ft::I16(), fluir::Operator::PLUS, &ft::I16(), &ft::I16()},
                                           tuple{&ft::I16(), fluir::Operator::MINUS, &ft::I16(), &ft::I16()},
                                           tuple{&ft::I16(), fluir::Operator::STAR, &ft::I16(), &ft::I16()},
                                           tuple{&ft::I16(), fluir::Operator::SLASH, &ft::I16(), &ft::I16()},
                                           tuple{&ft::I16(), fluir::Operator::PLUS, nullptr, &ft::I16()},
                                           tuple{&ft::I16(), fluir::Operator::MINUS, nullptr, &ft::I16()},
                                           tuple{&ft::I16(), fluir::Operator::PLUS_PLUS, nullptr, &ft::I16()},
                                           tuple{&ft::I16(), fluir::Operator::MINUS_MINUS, nullptr, &ft::I16()},
                                           tuple{&ft::I32(), fluir::Operator::PLUS, &ft::I32(), &ft::I32()},
                                           tuple{&ft::I32(), fluir::Operator::MINUS, &ft::I32(), &ft::I32()},
                                           tuple{&ft::I32(), fluir::Operator::STAR, &ft::I32(), &ft::I32()},
                                           tuple{&ft::I32(), fluir::Operator::SLASH, &ft::I32(), &ft::I32()},
                                           tuple{&ft::I32(), fluir::Operator::PLUS, nullptr, &ft::I32()},
                                           tuple{&ft::I32(), fluir::Operator::MINUS, nullptr, &ft::I32()},
                                           tuple{&ft::I32(), fluir::Operator::PLUS_PLUS, nullptr, &ft::I32()},
                                           tuple{&ft::I32(), fluir::Operator::MINUS_MINUS, nullptr, &ft::I32()},
                                           tuple{&ft::I64(), fluir::Operator::PLUS, &ft::I64(), &ft::I64()},
                                           tuple{&ft::I64(), fluir::Operator::MINUS, &ft::I64(), &ft::I64()},
                                           tuple{&ft::I64(), fluir::Operator::STAR, &ft::I64(), &ft::I64()},
                                           tuple{&ft::I64(), fluir::Operator::SLASH, &ft::I64(), &ft::I64()},
                                           tuple{&ft::I64(), fluir::Operator::PLUS, nullptr, &ft::I64()},
                                           tuple{&ft::I64(), fluir::Operator::MINUS, nullptr, &ft::I64()},
                                           tuple{&ft::I64(), fluir::Operator::PLUS_PLUS, nullptr, &ft::I64()},
                                           tuple{&ft::I64(), fluir::Operator::MINUS_MINUS, nullptr, &ft::I64()},
                                           tuple{&ft::U8(), fluir::Operator::PLUS, &ft::U8(), &ft::U8()},
                                           tuple{&ft::U8(), fluir::Operator::MINUS, &ft::U8(), &ft::U8()},
                                           tuple{&ft::U8(), fluir::Operator::STAR, &ft::U8(), &ft::U8()},
                                           tuple{&ft::U8(), fluir::Operator::SLASH, &ft::U8(), &ft::U8()},
                                           tuple{&ft::U8(), fluir::Operator::PLUS, nullptr, &ft::U8()},
                                           tuple{&ft::U8(), fluir::Operator::PLUS_PLUS, nullptr, &ft::U8()},
                                           tuple{&ft::U8(), fluir::Operator::MINUS_MINUS, nullptr, &ft::U8()},
                                           tuple{&ft::U16(), fluir::Operator::PLUS, &ft::U16(), &ft::U16()},
                                           tuple{&ft::U16(), fluir::Operator::MINUS, &ft::U16(), &ft::U16()},
                                           tuple{&ft::U16(), fluir::Operator::STAR, &ft::U16(), &ft::U16()},
                                           tuple{&ft::U16(), fluir::Operator::SLASH, &ft::U16(), &ft::U16()},
                                           tuple{&ft::U16(), fluir::Operator::PLUS, nullptr, &ft::U16()},
                                           tuple{&ft::U16(), fluir::Operator::PLUS_PLUS, nullptr, &ft::U16()},
                                           tuple{&ft::U16(), fluir::Operator::MINUS_MINUS, nullptr, &ft::U16()},
                                           tuple{&ft::U32(), fluir::Operator::PLUS, &ft::U32(), &ft::U32()},
                                           tuple{&ft::U32(), fluir::Operator::MINUS, &ft::U32(), &ft::U32()},
                                           tuple{&ft::U32(), fluir::Operator::STAR, &ft::U32(), &ft::U32()},
                                           tuple{&ft::U32(), fluir::Operator::SLASH, &ft::U32(), &ft::U32()},
                                           tuple{&ft::U32(), fluir::Operator::PLUS, nullptr, &ft::U32()},
                                           tuple{&ft::U32(), fluir::Operator::PLUS_PLUS, nullptr, &ft::U32()},
                                           tuple{&ft::U32(), fluir::Operator::MINUS_MINUS, nullptr, &ft::U32()},
                                           tuple{&ft::U64(), fluir::Operator::PLUS, &ft::U64(), &ft::U64()},
                                           tuple{&ft::U64(), fluir::Operator::MINUS, &ft::U64(), &ft::U64()},
                                           tuple{&ft::U64(), fluir::Operator::STAR, &ft::U64(), &ft::U64()},
                                           tuple{&ft::U64(), fluir::Operator::SLASH, &ft::U64(), &ft::U64()},
                                           tuple{&ft::U64(), fluir::Operator::PLUS, nullptr, &ft::U64()},
                                           tuple{&ft::U64(), fluir::Operator::PLUS_PLUS, nullptr, &ft::U64()},
                                           tuple{&ft::U64(), fluir::Operator::MINUS_MINUS, nullptr, &ft::U64()}));

class TestBuiltinCasts : public ::testing::TestWithParam<std::tuple<bool, ft::Type const*, ft::Type const*>> {
 public:
  fluir::types::SymbolTable uut;

  void SetUp() override { fluir::types::instantiateBuiltinCasts(uut); }
};

TEST_P(TestBuiltinCasts, Test) {
  auto& [isImplicit, source, target] = GetParam();

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
                         ::testing::Values(tuple{true, &ft::I8(), &ft::I16()},
                                           tuple{true, &ft::I8(), &ft::I32()},
                                           tuple{true, &ft::I8(), &ft::I64()},
                                           tuple{true, &ft::I8(), &ft::F64()},
                                           tuple{true, &ft::I16(), &ft::I32()},
                                           tuple{true, &ft::I16(), &ft::I64()},
                                           tuple{true, &ft::I16(), &ft::F64()},
                                           tuple{true, &ft::I32(), &ft::I64()},
                                           tuple{true, &ft::I32(), &ft::F64()},
                                           tuple{true, &ft::I64(), &ft::F64()},
                                           tuple{true, &ft::U8(), &ft::U16()},
                                           tuple{true, &ft::U8(), &ft::U32()},
                                           tuple{true, &ft::U8(), &ft::U64()},
                                           tuple{true, &ft::U8(), &ft::F64()},
                                           tuple{true, &ft::U16(), &ft::U32()},
                                           tuple{true, &ft::U16(), &ft::U64()},
                                           tuple{true, &ft::U16(), &ft::F64()},
                                           tuple{true, &ft::U32(), &ft::U64()},
                                           tuple{true, &ft::U32(), &ft::F64()},
                                           tuple{true, &ft::U64(), &ft::F64()}));
