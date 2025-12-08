#include "compiler/types/type.hpp"

#include <gtest/gtest.h>

TEST(TestType, StoreAndRetrieveName) {
  std::string expected = "test_type";

  fluir::types::Type uut{expected};

  EXPECT_EQ(expected, uut.name());
}

TEST(TestType, EqualityInequality) {
  fluir::types::Type uut1{"test_type"};
  fluir::types::Type uut2{"test_type"};
  fluir::types::Type uut3{"test_type2"};

  EXPECT_EQ(uut1, uut2);
  EXPECT_NE(uut1, uut3);
  EXPECT_NE(uut2, uut3);
}
