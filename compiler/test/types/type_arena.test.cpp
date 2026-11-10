#include "compiler/types/type_arena.hpp"

#include <gtest/gtest.h>

namespace ft = fluir::types;

class TestTypeArenaBuiltins : public ::testing::TestWithParam<fluir::types::Type const*> {
 public:
  ft::TypeArena uut;
};

TEST_P(TestTypeArenaBuiltins, Test) {
  auto& expected = GetParam();

  auto actual = uut.findTypeNamed(expected->name());

  ASSERT_TRUE(actual);
  EXPECT_EQ(expected, actual);
}

INSTANTIATE_TEST_SUITE_P(,
                         TestTypeArenaBuiltins,
                         ::testing::Values(&ft::F64(),
                                           &ft::I8(),
                                           &ft::I16(),
                                           &ft::I32(),
                                           &ft::I64(),
                                           &ft::U8(),
                                           &ft::U16(),
                                           &ft::U32(),
                                           &ft::U64(),
                                           &ft::BOOL(),
                                           &ft::EMPTY(),
                                           &ft::MAGIC_ANY_TYPE()));

TEST(TestTypeArena, AddProductType) {
  fluir::types::TypeArena uut;

  const auto actual = uut.addProduct({&ft::BOOL(), &ft::I64()});

  ASSERT_TRUE(actual->is<fluir::types::Product>());
  const auto* concrete = actual->as<fluir::types::Product>();
  EXPECT_EQ(&ft::BOOL(), concrete->at(0));
  EXPECT_EQ(&ft::I64(), concrete->at(1));
}

TEST(TestTypeArena, ProductTypesGetCorrectName) {
  fluir::types::TypeArena uut;
  std::string expected = "(BOOL,I64)";

  const auto actual = uut.addProduct({&ft::BOOL(), &ft::I64()});

  ASSERT_TRUE(actual->is<fluir::types::Product>());
  EXPECT_EQ(expected, actual->name());
}

TEST(TestTypeArena, AddProductsStructurallySame) {
  fluir::types::TypeArena uut;

  const auto actual1 = uut.addProduct({&ft::BOOL(), &ft::I64()});
  const auto actual2 = uut.addProduct({&ft::BOOL(), &ft::I64()});

  EXPECT_EQ(actual1, actual2);
}
