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
    void report(fd::Code, const fluir::Coordinate&, fd::AtWhat, std::string_view msg) override {
      ++reportedCount;
      lastExtraMessage = msg;
    }
  };
}  // namespace

TEST(TestDiagnosticSink, ThrowOnErrorEmit) {
  TestSink uut;
  EXPECT_THROW(uut.emit(fd::Code::GENERIC_ERROR, {0, 1, 2}, fd::AtWhat::LINE), fd::PanicMode);
}

TEST(TestDiagnosticSink, NoThrowOnWarningEmit) {
  TestSink uut;
  EXPECT_NO_THROW(uut.emit(fd::Code::GENERIC_WARNING, {0, 1, 2}, fd::AtWhat::LINE));
}

TEST(TestDiagnosticSink, NoThrowOnNoteEmit) {
  TestSink uut;
  EXPECT_NO_THROW(uut.emit(fd::Code::GENERIC_NOTE, {0, 1, 2}, fd::AtWhat::LINE));
}

TEST(TestDiagnosticSink, ThrowOnErrorEmitWithExtraMsg) {
  TestSink uut;
  EXPECT_THROW(uut.emit(fd::Code::GENERIC_ERROR, {1, 4, 5}, fd::AtWhat::NODE, "msg"), fd::PanicMode);
}

TEST(TestDiagnosticSink, NoThrowOnWarningEmitWithExtraMsg) {
  TestSink uut;
  EXPECT_NO_THROW(uut.emit(fd::Code::GENERIC_WARNING, {1, 4, 5}, fd::AtWhat::NODE, "msg"));
}

TEST(TestDiagnosticSink, NoThrowOnNoteEmitWithExtraMsg) {
  TestSink uut;
  EXPECT_NO_THROW(uut.emit(fd::Code::GENERIC_NOTE, {1, 4, 5}, fd::AtWhat::NODE, "msg"));
}

TEST(TestDiagnosticSink, SynchronizeStopsPanic) {
  TestSink uut;
  FLUIR_SYNCHRONIZE_PANIC(uut) { uut.emit(fd::Code::GENERIC_ERROR, {1, 4, 5}, fd::AtWhat::NODE); };
}

TEST(TestDiagnosticSink, ReportsDiagnosticsFirst) {
  TestSink uut;
  FLUIR_SYNCHRONIZE_PANIC(uut) { uut.emit(fd::Code::GENERIC_ERROR, {1, 4, 5}, fd::AtWhat::NODE); };

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

  uut.emit(fd::Code::GENERIC_WARNING, {1, 4, 5}, fd::AtWhat::NODE, "Error No. {}, and {}.", 12, "hello there");

  EXPECT_EQ(expected, uut.lastExtraMessage);
}
