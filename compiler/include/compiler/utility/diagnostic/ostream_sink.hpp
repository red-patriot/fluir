#ifndef FLUIR_COMPILER_UTILITY_DIAGNOSTIC_OSTREAM_SINK_HPP
#define FLUIR_COMPILER_UTILITY_DIAGNOSTIC_OSTREAM_SINK_HPP

#include <ostream>

#include "compiler/utility/diagnostic/sink.hpp"

namespace fluir::diagnostic {
  /** Prints error messages as strings to a std::ostream*/
  class OstreamSink : public Sink {
   public:
    explicit OstreamSink(std::ostream& os);
    OstreamSink(OstreamSink const&) = delete;
    OstreamSink& operator=(OstreamSink const&) = delete;
    OstreamSink(OstreamSink&&) = delete;
    OstreamSink& operator=(OstreamSink&&) = delete;
    ~OstreamSink() override = default;

    void operator()(const int& lineNo);
    void operator()(const FullID& id);

   private:
    std::ostream& os_;

    void report(Code code,
                const std::filesystem::path& file,
                const Sink::ErrorLocation& location,
                std::string_view msg) override;

    void printLocation(const Sink::ErrorLocation& location);
  };

  extern OstreamSink COUT_SINK;
}  // namespace fluir::diagnostic

#endif
