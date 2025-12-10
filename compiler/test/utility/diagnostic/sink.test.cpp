#include "compiler/utility/diagnostic/sink.hpp"

#include <vector>

#include <gtest/gtest.h>

#include "compiler/utility/diagnostic/internal_error.hpp"

namespace fd = fluir::diagnostic;

namespace {
  struct Dummy { };

  class LoggingTestSink : public fd::Sink<Dummy> {
   public:
    struct Data {
      fd::Code code;
      std::string_view msg;
    };
    std::vector<Data> reported;

   private:
    void report(fd::Code code, std::string_view msg, const Dummy&) override { reported.push_back({code, msg}); }
  };
}  // namespace

TEST(TestDiagnosticSink, ThrowOnErrorEmit) {
  LoggingTestSink uut;
  EXPECT_THROW(uut.emit(fd::Code::GENERIC_ERROR, Dummy{}), fd::PanicMode);
}

TEST(TestDiagnosticSink, NoThrowOnWarningEmit) {
  LoggingTestSink uut;
  EXPECT_NO_THROW(uut.emit(fd::Code::GENERIC_WARNING, Dummy{}));
}

TEST(TestDiagnosticSink, NoThrowOnNoteEmit) {
  LoggingTestSink uut;
  EXPECT_NO_THROW(uut.emit(fd::Code::GENERIC_NOTE, Dummy{}));
}

TEST(TestDiagnosticSink, ThrowOnErrorEmitWithExtraMsg) {
  LoggingTestSink uut;
  EXPECT_THROW(uut.emit(fd::Code::GENERIC_ERROR, "msg", Dummy{}), fd::PanicMode);
}

TEST(TestDiagnosticSink, NoThrowOnWarningEmitWithExtraMsg) {
  LoggingTestSink uut;
  EXPECT_NO_THROW(uut.emit(fd::Code::GENERIC_WARNING, "msg", Dummy{}));
}

TEST(TestDiagnosticSink, NoThrowOnNoteEmitWithExtraMsg) {
  LoggingTestSink uut;
  EXPECT_NO_THROW(uut.emit(fd::Code::GENERIC_NOTE, "msg", Dummy{}));
}

TEST(TestDiagnosticSink, SynchronizeStopsPanic) {
  LoggingTestSink uut;
  FLUIR_SYNCHRONIZE_PANIC(uut) { uut.emit(fd::Code::GENERIC_ERROR, Dummy{}); };
}

TEST(TestDiagnosticSink, ReportsDiagnosticsFirst) {
  LoggingTestSink uut;
  FLUIR_SYNCHRONIZE_PANIC(uut) { uut.emit(fd::Code::GENERIC_ERROR, Dummy{}); };

  EXPECT_EQ(1, uut.reported.size());
}

TEST(TestDiagnosticSink, ThrowsOnInternalError) {
  LoggingTestSink uut;
  EXPECT_TRUE((std::derived_from<fd::InternalError, std::exception>));
  EXPECT_THROW(fd::emitInternalError("msg"), fd::InternalError);
}
