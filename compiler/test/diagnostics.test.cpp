#include <gtest/gtest.h>

#include "fluir/compiler/diagnostic.hpp"

TEST(TestDiagnostics, CanEmitError) {
  fluir::compiler::Diagnostic expected{
      .level = fluir::compiler::Diagnostic::ERROR,
      .message = "Something went wrong."};

  fluir::compiler::DiagnosticCollection diags;

  fluir::compiler::emitError(diags, "Something went wrong.");

  EXPECT_TRUE(diags.size() == 1);
  EXPECT_EQ(expected, diags.at(0));
}

TEST(TestDiagnostics, CanEmitFormattedError) {
  fluir::compiler::Diagnostic expected{
      .level = fluir::compiler::Diagnostic::ERROR,
      .message = "Something went wrong at (7)."};

  fluir::compiler::DiagnosticCollection diags;

  fluir::compiler::emitError(diags, "Something went wrong at ({}).", 7);

  EXPECT_TRUE(diags.size() == 1);
  EXPECT_EQ(expected, diags.at(0));
}
