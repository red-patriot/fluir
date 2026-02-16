#ifndef FLUIR_COMPILER_UTILITY_DIAGNOSTIC_COLORED_STDOUT_SINK_HPP
#define FLUIR_COMPILER_UTILITY_DIAGNOSTIC_COLORED_STDOUT_SINK_HPP

#include "compiler/utility/diagnostic/sink.hpp"

namespace fluir::diagnostic {
  /** Prints colored messages to stdout/stderr */
  class ColoredStdoutSink : public Sink {
   public:
    void operator()(const int& lineNo) const;
    void operator()(const FullID& id) const;

   private:
    void report(Code code,
                const std::filesystem::path& file,
                const ErrorLocation& location,
                std::string_view extraMsg) override;

    void printCode(Code code) const;
    void printLocation(const ErrorLocation& location) const;
  };

  ColoredStdoutSink& getColoredStdoutSink();
}  // namespace fluir::diagnostic

#endif
