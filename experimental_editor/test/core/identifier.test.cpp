#include "editor/core/identifier.hpp"

#include <gtest/gtest.h>

namespace {

  using fluir::editor::isValidIdentifier;

  TEST(Identifier, AcceptsLettersDigitsAndUnderscoresNotLeadingWithADigit) {
    EXPECT_TRUE(isValidIdentifier("f"));
    EXPECT_TRUE(isValidIdentifier("_"));
    EXPECT_TRUE(isValidIdentifier("_x1"));
    EXPECT_TRUE(isValidIdentifier("add_two"));
    EXPECT_TRUE(isValidIdentifier("A9"));
  }

  TEST(Identifier, RejectsEmptyLeadingDigitsAndOtherCharacters) {
    EXPECT_FALSE(isValidIdentifier(""));
    EXPECT_FALSE(isValidIdentifier("1f"));
    EXPECT_FALSE(isValidIdentifier("a-b"));
    EXPECT_FALSE(isValidIdentifier("a b"));
    EXPECT_FALSE(isValidIdentifier("é"));
    EXPECT_FALSE(isValidIdentifier("a.b"));
  }

}  // namespace
