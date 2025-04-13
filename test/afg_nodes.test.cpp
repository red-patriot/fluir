#include <gtest/gtest.h>

#include "fluir/compiler/afg.hpp"

namespace fafg = fluir::afg;

TEST(TestConstant, MakeFloat) {
  auto actual = fafg::makeDoubleConstant(1.0);

  EXPECT_EQ(fafg::Type::DOUBLE_FP, actual.type());
  EXPECT_DOUBLE_EQ(1.0, actual.as<fafg::Type::DOUBLE_FP>());
}
