#include "compiler/utility/diagnostic/sink.hpp"

#include <gtest/gtest.h>

#include "compiler/utility/diagnostic/internal_error.hpp"
#include "compiler/utility/diagnostic/sink.hpp"

namespace fd = fluir::diagnostic;

namespace {
  class TestSink : public fd::Sink {
   public:
    size_t reportedCount{0};
    std::string lastExtraMessage{};

   private:
    void report(fd::Code, const fd::Sink::ErrorLocation&, std::string_view msg) override {
      ++reportedCount;
      lastExtraMessage = msg;
    }
  };
}  // namespace

TEST(TestDiagnosticSink, ThrowOnErrorEmit) {
  TestSink uut;
  EXPECT_THROW(uut.emitAtElement(fd::Code::GENERIC_ERROR, {12}), fd::PanicMode);
}

TEST(TestDiagnosticSink, NoThrowOnWarningEmit) {
  TestSink uut;
  EXPECT_NO_THROW(uut.emitAtElement(fd::Code::GENERIC_WARNING, {3, 4, 5, 6}));
}

TEST(TestDiagnosticSink, NoThrowOnNoteEmit) {
  TestSink uut;
  EXPECT_NO_THROW(uut.emitAtElement(fd::Code::GENERIC_NOTE, {2}));
}

TEST(TestDiagnosticSink, ThrowOnErrorEmitWithExtraMsg) {
  TestSink uut;
  EXPECT_THROW(uut.emitAtLine(fd::Code::GENERIC_ERROR, 18, "msg"), fd::PanicMode);
}

TEST(TestDiagnosticSink, NoThrowOnWarningEmitWithExtraMsg) {
  TestSink uut;
  EXPECT_NO_THROW(uut.emitAtLine(fd::Code::GENERIC_WARNING, 18, "msg"));
}

TEST(TestDiagnosticSink, NoThrowOnNoteEmitWithExtraMsg) {
  TestSink uut;
  EXPECT_NO_THROW(uut.emitAtLine(fd::Code::GENERIC_NOTE, 18, "msg"));
}

TEST(TestDiagnosticSink, SynchronizeStopsPanic) {
  TestSink uut;
  FLUIR_SYNCHRONIZE_PANIC(uut) { uut.emitAtLine(fd::Code::GENERIC_ERROR, 0); };
}

TEST(TestDiagnosticSink, ReportsDiagnosticsFirst) {
  TestSink uut;
  FLUIR_SYNCHRONIZE_PANIC(uut) { uut.emitAtLine(fd::Code::GENERIC_ERROR, 0); };

  EXPECT_EQ(1, uut.reportedCount);
}

TEST(TestDiagnosticSink, ThrowsOnInternalError) {
  TestSink uut;
  EXPECT_TRUE((std::derived_from<fd::InternalError, std::exception>));
  EXPECT_THROW(fd::emitInternalError("msg"), fd::InternalError);
}

TEST(TestDiagnosticSink, SendsFormattedMsg) {
  TestSink uut;
  std::string expected = "Error No. 12, and hello there.";

  uut.emitAtLine(fd::Code::GENERIC_WARNING, 7, "Error No. {}, and {}.", 12, "hello there");

  EXPECT_EQ(expected, uut.lastExtraMessage);
}
