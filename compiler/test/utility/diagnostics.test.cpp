#include <gtest/gtest.h>

import fluir.utility.diagnostics;

namespace {
  class TestingLocation : public fluir::Diagnostic::Location {
   public:
    explicit TestingLocation(int line) : line_(line) { }

    std::string str() const override { return "at " + std::to_string(line_); }

    int line_;
  };
}  // namespace

TEST(TestDiagnostics, EmitError) {
  fluir::Diagnostic expected{
    fluir::Diagnostic::ERROR,
    "Something went wrong",
  };

  fluir::Diagnostics actual;
  actual.emitError("Something went wrong");

  EXPECT_EQ(expected, actual.back());
}

TEST(TestDiagnostics, EmitErrorWithLocation) {
  fluir::Diagnostic expected{fluir::Diagnostic::ERROR, "Something went wrong"};

  fluir::Diagnostics actual;
  actual.emitError("Something went wrong", std::make_unique<TestingLocation>(4));

  EXPECT_EQ(expected, actual.back());
  auto location = dynamic_cast<TestingLocation*>(actual.back().where.get());
  EXPECT_TRUE(location);
  EXPECT_EQ(4, location->line_);
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

TEST(TestDiagnostics, EmitWarningWithLocation) {
  fluir::Diagnostic expected{fluir::Diagnostic::WARNING, "Something is worrying..."};

  fluir::Diagnostics actual;
  actual.emitWarning("Something is worrying...", std::make_unique<TestingLocation>(7));

  EXPECT_EQ(expected, actual.back());
  auto location = dynamic_cast<TestingLocation*>(actual.back().where.get());
  EXPECT_TRUE(location);
  EXPECT_EQ(7, location->line_);
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

TEST(TestDiagnostics, EmitNoteWithLocation) {
  fluir::Diagnostic expected{
    fluir::Diagnostic::NOTE,
    "You may be interested in this",
  };

  fluir::Diagnostics actual;
  actual.emitNote("You may be interested in this", std::make_unique<TestingLocation>(-54));

  EXPECT_EQ(expected, actual.back());
  auto location = dynamic_cast<TestingLocation*>(actual.back().where.get());
  EXPECT_TRUE(location);
  EXPECT_EQ(-54, location->line_);
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

TEST(TestDiagnostics, EmitInternalErrorWithLocation) {
  fluir::Diagnostic expected{
    fluir::Diagnostic::INTERNAL_ERROR,
    "This is likely a bug.",
  };

  fluir::Diagnostics actual;
  actual.emitInternalError("This is likely a bug.", std::make_unique<TestingLocation>(-54));

  EXPECT_EQ(expected, actual.back());
  auto location = dynamic_cast<TestingLocation*>(actual.back().where.get());
  EXPECT_TRUE(location);
  EXPECT_EQ(-54, location->line_);
}

TEST(TestDiagnostics, StringifyError) {
  std::string expected = "[ERROR]: A problem occurred.";

  fluir::Diagnostic diagnostic{fluir::Diagnostic::ERROR, "A problem occurred."};

  auto actual = fluir::toString(diagnostic);

  EXPECT_EQ(expected, actual);
}

TEST(TestDiagnostics, StringifyErrorWithLocation) {
  std::string expected = "[ERROR] at 4: A problem occurred.";

  fluir::Diagnostic diagnostic{fluir::Diagnostic::ERROR, "A problem occurred.", std::make_unique<TestingLocation>(4)};

  auto actual = fluir::toString(diagnostic);

  EXPECT_EQ(expected, actual);
}

TEST(TestDiagnostics, StringifyWarning) {
  std::string expected = "[WARNING]: An issue.";

  fluir::Diagnostic diagnostic{fluir::Diagnostic::WARNING, "An issue."};

  auto actual = fluir::toString(diagnostic);

  EXPECT_EQ(expected, actual);
}

TEST(TestDiagnostics, StringifyWarningWithLocation) {
  std::string expected = "[WARNING] at -59: An issue.";

  fluir::Diagnostic diagnostic{fluir::Diagnostic::WARNING, "An issue.", std::make_unique<TestingLocation>(-59)};

  auto actual = fluir::toString(diagnostic);

  EXPECT_EQ(expected, actual);
}

TEST(TestDiagnostics, StringifyNote) {
  std::string expected = "[NOTE]: Some important information.";

  fluir::Diagnostic diagnostic{fluir::Diagnostic::NOTE, "Some important information."};

  auto actual = fluir::toString(diagnostic);

  EXPECT_EQ(expected, actual);
}

TEST(TestDiagnostics, StringifyNoteWithLocation) {
  std::string expected = "[NOTE] at 112: Some important information.";

  fluir::Diagnostic diagnostic{
    fluir::Diagnostic::NOTE, "Some important information.", std::make_unique<TestingLocation>(112)};

  auto actual = fluir::toString(diagnostic);

  EXPECT_EQ(expected, actual);
}

TEST(TestDiagnostics, StringifyInternalError) {
  std::string expected = "[INTERNAL ERROR]: If you see this, file a bug.";

  fluir::Diagnostic diagnostic{fluir::Diagnostic::INTERNAL_ERROR, "If you see this, file a bug."};

  auto actual = fluir::toString(diagnostic);

  EXPECT_EQ(expected, actual);
}

TEST(TestDiagnostics, StringifyInternalErrorWithLocation) {
  std::string expected = "[INTERNAL ERROR] at 9: If you see this, file a bug.";

  fluir::Diagnostic diagnostic{
    fluir::Diagnostic::INTERNAL_ERROR, "If you see this, file a bug.", std::make_unique<TestingLocation>(9)};

  auto actual = fluir::toString(diagnostic);

  EXPECT_EQ(expected, actual);
}

TEST(TestDiagnostics, DetectLevelNote) {
  fluir::Diagnostic diag{fluir::Diagnostic::NOTE, ""};

  EXPECT_FALSE(fluir::isError(diag));
}

TEST(TestDiagnostics, DetectLevelNWarning) {
  fluir::Diagnostic diag{fluir::Diagnostic::WARNING, ""};

  EXPECT_FALSE(fluir::isError(diag));
}

TEST(TestDiagnostics, DetectLevelError) {
  fluir::Diagnostic diag{fluir::Diagnostic::ERROR, ""};

  EXPECT_TRUE(fluir::isError(diag));
}

TEST(TestDiagnostics, DetectLevelInternalError) {
  fluir::Diagnostic diag{fluir::Diagnostic::INTERNAL_ERROR, ""};

  EXPECT_TRUE(fluir::isError(diag));
}
