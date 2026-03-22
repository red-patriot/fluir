#include "options.hpp"

#include <gtest/gtest.h>

namespace fluir::lsp {
  TEST(Options, ParsesPortShortFlag) {
    auto result = parseArgs({"fluir.lsp", "-p", "8080"});
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->port, 8080);
    EXPECT_EQ(result->logLevel, LogLevel::NORMAL);
  }

  TEST(Options, ParsesPortLongFlag) {
    auto result = parseArgs({"fluir.lsp", "--port", "9000"});
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->port, 9000);
  }

  TEST(Options, ParsesVerboseFlag) {
    auto result = parseArgs({"fluir.lsp", "-p", "8080", "-v"});
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->port, 8080);
    EXPECT_EQ(result->logLevel, LogLevel::DEBUG);
  }

  TEST(Options, MissingPortReturnsNullopt) {
    auto result = parseArgs({"fluir.lsp"});
    EXPECT_FALSE(result.has_value());
  }

  TEST(Options, DefaultLogLevelIsNormal) {
    auto result = parseArgs({"fluir.lsp", "-p", "5000"});
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->logLevel, LogLevel::NORMAL);
  }
}  // namespace fluir::lsp
