#include "editor/core/choices.hpp"

#include <string>
#include <vector>

#include <gtest/gtest.h>

// The fixed menus a Choice field offers until module-aware intelligence lands.

TEST(Choices, BinaryOperators) {
  const std::vector<std::string> expected{"+", "-", "*", "/", "==", "!=", ">=", "<=", ">", "<", "&&", "||"};
  EXPECT_EQ(fluir::editor::binaryOperators(), expected);
}

TEST(Choices, UnaryOperators) {
  const std::vector<std::string> expected{"+", "-", "++", "--", "!"};
  EXPECT_EQ(fluir::editor::unaryOperators(), expected);
}

TEST(Choices, BuiltinTypes) {
  const std::vector<std::string> expected{"F64", "I8", "I16", "I32", "I64", "U8", "U16", "U32", "U64", "BOOL"};
  EXPECT_EQ(fluir::editor::builtinTypes(), expected);
}
