#include "compiler/types/type.hpp"

#include <gtest/gtest.h>

#include "compiler/types/typeid.hpp"

namespace ft = fluir::types;

TEST(TestType, StoreAndRetrieveName) {
  std::string expected = "test_type";

  ft::Type uut{expected};

  EXPECT_EQ(expected, uut.name());
}

TEST(TestType, EqualityInequality) {
  ft::Type uut1{"test_type"};
  ft::Type uut2{"test_type"};
  ft::Type uut3{"test_type2"};

  EXPECT_EQ(uut1, uut2);
  EXPECT_NE(uut1, uut3);
  EXPECT_NE(uut2, uut3);
}

TEST(TestType, ProductTypeDef) {
  ft::Product uut{"test", {ft::TypeID::ID_F64, ft::TypeID::ID_I32, ft::TypeID::ID_BOOL}};

  EXPECT_TRUE(uut.is<ft::Product>());

  EXPECT_EQ(ft::TypeID::ID_F64, uut.at(0));
  EXPECT_EQ(ft::TypeID::ID_I32, uut.at(1));
  EXPECT_EQ(ft::TypeID::ID_BOOL, uut.at(2));
}

TEST(TestType, AnonymousProductName) {
  std::string expected = "__product_1_2_12";

  auto actual = ft::Product::anonymousName({ft::TypeID{1}, ft::TypeID{2}, ft::TypeID{12}});

  EXPECT_EQ(expected, actual);
}

TEST(TestType, AnonymousProduct) {
  std::vector<ft::TypeID> ids{ft::TypeID{17}, ft::TypeID{5000}};
  ft::Product expected{"__product_17_5000", ids};

  auto actual = ft::Product::anonymous(ids);

  EXPECT_EQ(expected, actual);
}
