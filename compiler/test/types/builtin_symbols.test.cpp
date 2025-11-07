#include "compiler/types/builtin_symbols.hpp"

#include <gtest/gtest.h>

#include "compiler/types/symbol_table.hpp"
#include "compiler/types/type.hpp"

namespace ft = fluir::types;

class TestBuiltinTypes : public ::testing::TestWithParam<fluir::types::Type> {
 public:
  fluir::types::SymbolTable uut;

  void SetUp() override { fluir::types::instantiateBuiltinTypes(uut); }
};

TEST_P(TestBuiltinTypes, Test) {
  auto& expected = GetParam();

  auto actual = uut.getType(expected.name());

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
