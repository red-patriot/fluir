#include "compiler/utility/diagnostics.hpp"

#include <gtest/gtest.h>

namespace {
  class TestingLocation : public fluir::Diagnostic::Location {
   public:
    explicit TestingLocation(int line) :
        line_(line) { }

    std::string str() const override {
      return std::to_string(line_);
    }

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
  fluir::Diagnostic expected{
      fluir::Diagnostic::ERROR,
      "Something went wrong"};

  fluir::Diagnostics actual;
  actual.emitError("Something went wrong",
                   std::make_unique<TestingLocation>(4));

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
  fluir::Diagnostic expected{
      fluir::Diagnostic::WARNING,
      "Something is worrying..."};

  fluir::Diagnostics actual;
  actual.emitWarning("Something is worrying...",
                     std::make_unique<TestingLocation>(7));

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
  actual.emitNote("You may be interested in this",
                  std::make_unique<TestingLocation>(-54));

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
  actual.emitInternalError("This is likely a bug.",
                           std::make_unique<TestingLocation>(-54));

  EXPECT_EQ(expected, actual.back());
  auto location = dynamic_cast<TestingLocation*>(actual.back().where.get());
  EXPECT_TRUE(location);
  EXPECT_EQ(-54, location->line_);
}
