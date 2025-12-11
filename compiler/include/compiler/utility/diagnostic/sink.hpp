#ifndef FLUIR_COMPILER_UTILITY_DIAGNOSTIC_SINK_HPP
#define FLUIR_COMPILER_UTILITY_DIAGNOSTIC_SINK_HPP

#include <stacktrace>
#include <string_view>
#include <variant>

#include <fmt/format.h>

#include "compiler/models/id.hpp"
#include "compiler/utility/diagnostic/code.hpp"
#include "compiler/utility/diagnostic/panic.hpp"

namespace fluir::diagnostic {
  /** A generic sink for emitting (user) diagnostics from the compiler */
  class Sink {
   public:
    virtual ~Sink() = default;

    /** Emits a diagnostic at the given line with an optional extra message for additional context.
     * Initiates a panic if an error is emitted.
     * Use the FLUIR_SYNCHRONIZE_PANIC macro to synchronize after a panic
     */
    template <typename... FmtArgs>
    void emitAtLine(Code code, int lineNo, fmt::format_string<FmtArgs...> extraMsg = "", FmtArgs&&... args) {
      report(code, lineNo, fmt::format(extraMsg, std::forward<FmtArgs>(args)...));
      if (code >= Code::GENERIC_ERROR) {
        startPanic();
      }
    }

    /** Emits a diagnostic at the element with the given fullID with an optional extra message for additional context.
     * Initiates a panic if an error is emitted.
     * Use the FLUIR_SYNCHRONIZE_PANIC macro to synchronize after a panic
     */
    template <typename... FmtArgs>
    void emitAtElement(Code code, const FullID& id, fmt::format_string<FmtArgs...> extraMsg = "", FmtArgs&&... args) {
      report(code, id, fmt::format(extraMsg, std::forward<FmtArgs>(args)...));
      if (code >= Code::GENERIC_ERROR) {
        startPanic();
      }
    }

   protected:
    /** Where an error can be emitted: a line number or element ID */
    using ErrorLocation = std::variant<int, FullID>;

    /** Report the given error to the sink location */
    virtual void report(Code diagnostic, const ErrorLocation&, std::string_view extraMsg) = 0;
  };
}  // namespace fluir::diagnostic

#endif
