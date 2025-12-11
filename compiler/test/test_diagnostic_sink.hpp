#ifndef FLUIR_COMPILER_TEST_TEST_DIAGNOSTICS_SINK_HPP
#define FLUIR_COMPILER_TEST_TEST_DIAGNOSTICS_SINK_HPP

#include <algorithm>

#include "compiler/utility/diagnostic/sink.hpp"

namespace fluir::test {

  class TestDiagnosticSink : public diagnostic::Sink {
   public:
    using ErrorLocation = diagnostic::Sink::ErrorLocation;

    struct DiagnosticData {
      fluir::diagnostic::Code code;
      ErrorLocation location;

      friend bool operator==(const DiagnosticData& lhs, const DiagnosticData& rhs) = default;
    };

    bool containsErrors() const {
      return std::ranges::any_of(
        emitted_, [](const DiagnosticData& data) { return data.code >= fluir::diagnostic::Code::GENERIC_ERROR; });
    }

    void clear() { emitted_.clear(); }
    const std::vector<DiagnosticData>& emitted() { return emitted_; }

   private:
    std::vector<DiagnosticData> emitted_;

    void report(diagnostic::Code code,
                const std::filesystem::path&,
                const ErrorLocation& location,
                std::string_view) override {
      emitted_.emplace_back(DiagnosticData{code, location});
    }
  };

}  // namespace fluir::test

#endif
