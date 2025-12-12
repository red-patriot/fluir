#include "compiler/utility/diagnostic/ostream_sink.hpp"

#include <iostream>

#include <fmt/format.h>

#include "compiler/utility/diagnostic/pretty_msg.hpp"
#include "compiler/utility/diagnostics.hpp"

namespace fluir::diagnostic {
  namespace {
    std::string_view levelOf(Code code) {
      if (code >= Code::GENERIC_ERROR) {
        return "[ERROR]";
      } else if (code >= Code::GENERIC_WARNING) {
        return "[WARNING]";
      } else {
        return "[NOTE]";
      }
    }
  }  // namespace

  OstreamSink::OstreamSink(std::ostream& os) : os_(&os) { }

  void OstreamSink::operator()(const int& lineNo) { (*os_) << fmt::format("at line {}", lineNo); }
  void OstreamSink::operator()(const FullID& id) { (*os_) << fmt::format(" at element {}", fmt::join(id, ":")); }

  void OstreamSink::report(Code code,
                           const std::filesystem::path& file,
                           const Sink::ErrorLocation& location,
                           std::string_view msg) {
    (*os_) << fmt::format("{} in '{}' ", levelOf(code), file.string());
    printLocation(location);
    (*os_) << "\n\t" << prettyMessage(code);
    if (!msg.empty()) {
      (*os_) << "\n\t" << msg;
    }
    (*os_) << '\n';
  }

  void OstreamSink::printLocation(const Sink::ErrorLocation& location) { std::visit(*this, location); }

  OstreamSink& getCoutSink() {
    static OstreamSink COUT_SINK{std::cout};
    return COUT_SINK;
  }
}  // namespace fluir::diagnostic
