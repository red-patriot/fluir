#include "vm/validate.hpp"

#include <bytecode/version.hpp>
#include <cstdint>
#include <gtest/gtest.h>

using std::tuple;

class TestValidateVersion : public ::testing::TestWithParam<fluir::Version> { };

TEST_P(TestValidateVersion, FailsOnNewVersion) {
  auto& given = GetParam();

  fluir::Version current{2, 3, 4};

  EXPECT_FALSE(fluir::checkVersionIsValid(current, given));
}

TEST_P(TestValidateVersion, SucceedsOnOldVersion) {
  auto& given = GetParam();

  fluir::Version current{3,2,1};

  EXPECT_TRUE(fluir::checkVersionIsValid(current, given));
}

INSTANTIATE_TEST_SUITE_P(,
                         TestValidateVersion,
                         ::testing::Values(fluir::Version{2, 3, 5},
                                           fluir::Version{2, 5, 1},
                                           fluir::Version{2, 5, 4},
                                           fluir::Version{3, 0, 0},
                                           fluir::Version{3, 2, 1}));