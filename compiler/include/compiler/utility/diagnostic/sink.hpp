#ifndef FLUIR_COMPILER_UTILITY_DIAGNOSTIC_SINK_HPP
#define FLUIR_COMPILER_UTILITY_DIAGNOSTIC_SINK_HPP

#include <stacktrace>
#include <string_view>

#include "compiler/utility/diagnostic/code.hpp"
#include "compiler/utility/diagnostic/panic.hpp"

namespace fluir::diagnostic {

  /** A generic sink for emitting (user) diagnostics from the compiler */
  template <typename LocationType>
  class Sink {
   public:
    using Location = LocationType;

    virtual ~Sink() = default;

    /** Emits a diagnostic at the given location.
     * Initiates a panic if an error is emitted.
     * Use the FLUIR_SYNCHRONIZE_PANIC macro to synchronize after a panic
     */
    void emit(Code code, const Location& where) {
      report(code, "", where);
      if (code >= Code::GENERIC_ERROR) {
        startPanic();
      }
    }

    /** Emits a diagnostic at the given location with an extra message for additional context.
     * Initiates a panic if an error is emitted.
     * Use the FLUIR_SYNCHRONIZE_PANIC macro to synchronize after a panic
     */
    void emit(Code code, std::string_view extraMsg, const Location& where) {
      report(code, extraMsg, where);
      if (code >= Code::GENERIC_ERROR) {
        startPanic();
      }
    }

   protected:
    /** Report the given error to the sink location */
    virtual void report(Code diagnostic, std::string_view extraMsg, const Location& where) = 0;
  };
}  // namespace fluir::diagnostic

#endif
