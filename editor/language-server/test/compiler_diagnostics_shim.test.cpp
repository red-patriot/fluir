#include "lsp/database/compiler_diagnostics_shim.hpp"

#include <compiler/utility/diagnostic/panic.hpp>
#include <compiler/utility/diagnostic/pretty_msg.hpp>
#include <gtest/gtest.h>

using fluir::diagnostic::Code;
using fluir::lsp::api::DiagnosticSeverity;
using fluir::lsp::api::ModuleDiagnostic;

class CompilerDiagnosticsShimTest : public ::testing::Test {
 protected:
  std::vector<ModuleDiagnostic> diagnostics_;
  fluir::lsp::CompilerDiagnosticsShim shim_{diagnostics_};
};

TEST_F(CompilerDiagnosticsShimTest, ErrorCodeMapsToDiagnosticError) {
  FLUIR_SYNCHRONIZE_PANIC(shim_) { shim_.emitAtElement(Code::GENERIC_ERROR, "test.fl", {1, 2}); };
  ASSERT_EQ(diagnostics_.size(), 1);
  EXPECT_EQ(diagnostics_[0].severity, DiagnosticSeverity::ERROR);
}

TEST_F(CompilerDiagnosticsShimTest, SpecificErrorCodeMapsToDiagnosticError) {
  FLUIR_SYNCHRONIZE_PANIC(shim_) { shim_.emitAtElement(Code::ERROR_MISSING_ELEMENT, "test.fl", {1}); };
  ASSERT_EQ(diagnostics_.size(), 1);
  EXPECT_EQ(diagnostics_[0].severity, DiagnosticSeverity::ERROR);
}

TEST_F(CompilerDiagnosticsShimTest, WarningCodeMapsToDiagnosticWarning) {
  shim_.emitAtElement(Code::GENERIC_WARNING, "test.fl", {1});
  ASSERT_EQ(diagnostics_.size(), 1);
  EXPECT_EQ(diagnostics_[0].severity, DiagnosticSeverity::WARNING);
}

TEST_F(CompilerDiagnosticsShimTest, NoteCodeMapsToDiagnosticInformation) {
  shim_.emitAtElement(Code::GENERIC_NOTE, "test.fl", {1});
  ASSERT_EQ(diagnostics_.size(), 1);
  EXPECT_EQ(diagnostics_[0].severity, DiagnosticSeverity::INFORMATION);
}

TEST_F(CompilerDiagnosticsShimTest, FullIDLocationPassedThrough) {
  fluir::FullID id{10, 20, 30};
  shim_.emitAtElement(Code::GENERIC_NOTE, "test.fl", id);
  ASSERT_EQ(diagnostics_.size(), 1);
  ASSERT_TRUE(std::holds_alternative<fluir::FullID>(diagnostics_[0].location));
  EXPECT_EQ(std::get<fluir::FullID>(diagnostics_[0].location), id);
}

TEST_F(CompilerDiagnosticsShimTest, LineNumberLocationBecomesEmptyFullID) {
  shim_.emitAtLine(Code::GENERIC_NOTE, "test.fl", 42);
  ASSERT_EQ(diagnostics_.size(), 1);
  ASSERT_TRUE(std::holds_alternative<fluir::FullID>(diagnostics_[0].location));
  EXPECT_TRUE(std::get<fluir::FullID>(diagnostics_[0].location).empty());
}

TEST_F(CompilerDiagnosticsShimTest, MessageCombinesPrettyMessageAndExtra) {
  shim_.emitAtElement(Code::GENERIC_NOTE, "test.fl", {1}, "extra detail");
  ASSERT_EQ(diagnostics_.size(), 1);
  auto expected = fluir::diagnostic::prettyMessage(Code::GENERIC_NOTE) + ": extra detail";
  EXPECT_EQ(diagnostics_[0].message, expected);
}

TEST_F(CompilerDiagnosticsShimTest, MessageWithoutExtraIsJustPrettyMessage) {
  shim_.emitAtElement(Code::GENERIC_NOTE, "test.fl", {1});
  ASSERT_EQ(diagnostics_.size(), 1);
  EXPECT_EQ(diagnostics_[0].message, fluir::diagnostic::prettyMessage(Code::GENERIC_NOTE));
}

TEST_F(CompilerDiagnosticsShimTest, MultipleDiagnosticsAccumulate) {
  shim_.emitAtElement(Code::GENERIC_NOTE, "test.fl", {1});
  shim_.emitAtElement(Code::GENERIC_WARNING, "test.fl", {2});
  FLUIR_SYNCHRONIZE_PANIC(shim_) { shim_.emitAtElement(Code::GENERIC_ERROR, "test.fl", {3}); };
  EXPECT_EQ(diagnostics_.size(), 3);
  EXPECT_EQ(diagnostics_[0].severity, DiagnosticSeverity::INFORMATION);
  EXPECT_EQ(diagnostics_[1].severity, DiagnosticSeverity::WARNING);
  EXPECT_EQ(diagnostics_[2].severity, DiagnosticSeverity::ERROR);
}
