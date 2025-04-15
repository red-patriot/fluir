#include "compiler/utility/diagnostics.hpp"

#include <gtest/gtest.h>

TEST(TestDiagnostics, EmitError) {
  fluir::Diagnostic expected{
      fluir::Diagnostic::ERROR,
      "Something went wrong",
  };

  fluir::Diagnostics actual;
  actual.emitError("Something went wrong");

  EXPECT_EQ(expected, actual.back());
}

TEST(TestDiagnostics, EmitWarning) {
  fluir::Diagnostic expected{
      fluir::Diagnostic::WARNING,
      "Something is worrying...",
  };

  fluir::Diagnostics actual;
  actual.emitWarning("Something is worrying...");

  EXPECT_EQ(expected, actual.back());
}

TEST(TestDiagnostics, EmitNote) {
  fluir::Diagnostic expected{
      fluir::Diagnostic::NOTE,
      "You may be interested in this",
  };

  fluir::Diagnostics actual;
  actual.emitNote("You may be interested in this");

  EXPECT_EQ(expected, actual.back());
}

TEST(TestDiagnostics, EmitInternalError) {
  fluir::Diagnostic expected{
      fluir::Diagnostic::INTERNAL_ERROR,
      "This is likely a bug.",
  };

  fluir::Diagnostics actual;
  actual.emitInternalError("This is likely a bug.");

  EXPECT_EQ(expected, actual.back());
}
