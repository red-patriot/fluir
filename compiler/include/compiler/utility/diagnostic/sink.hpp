#ifndef FLUIR_COMPILER_UTILITY_DIAGNOSTIC_SINK_HPP
#define FLUIR_COMPILER_UTILITY_DIAGNOSTIC_SINK_HPP

#include <stacktrace>
#include <string_view>

#include <fmt/format.h>

#include "compiler/models/location.hpp"
#include "compiler/utility/diagnostic/code.hpp"
#include "compiler/utility/diagnostic/panic.hpp"

namespace fluir::diagnostic {
  enum class AtWhat {
    LINE,
    NODE,
  };

  /** A generic sink for emitting (user) diagnostics from the compiler */
  class Sink {
   public:
    virtual ~Sink() = default;

    /** Emits a diagnostic at the given location with an optional extra message for additional context.
     * Initiates a panic if an error is emitted.
     * Use the FLUIR_SYNCHRONIZE_PANIC macro to synchronize after a panic
     */
    template <typename... FmtArgs>
    void emit(
      Code code, const Coordinate& where, AtWhat at, fmt::format_string<FmtArgs...> extraMsg = "", FmtArgs&&... args) {
      report(code, where, at, fmt::format(extraMsg, std::forward<FmtArgs>(args)...));
      if (code >= Code::GENERIC_ERROR) {
        startPanic();
      }
    }

   protected:
    /** Report the given error to the sink location */
    virtual void report(Code diagnostic, const Coordinate& where, AtWhat at, std::string_view extraMsg) = 0;
  };
}  // namespace fluir::diagnostic

#endif
