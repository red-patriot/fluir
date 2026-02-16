#include "compiler/utility/diagnostic/colored_stdout_sink.hpp"

#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include "compiler/utility/diagnostic/pretty_msg.hpp"

using fmt::color;
using fmt::fg;

namespace fluir::diagnostic {
  namespace {
    void printLevel(Code code) {
      if (isError(code)) {
        fmt::print(fg(color::red), "[ERROR]");
      } else if (isWarning(code)) {
        fmt::print(fg(color::golden_rod), "[WARNING]");
      } else {
        fmt::print(fg(color::sky_blue), "[NOTE]");
      }
    }
  }  // namespace

  void ColoredStdoutSink::operator()(const int& lineNo) const { fmt::print("at line {}", lineNo); }
  void ColoredStdoutSink::operator()(const FullID& id) const { fmt::print("at element {}", fmt::join(id, ":")); }

  void ColoredStdoutSink::report(Code code,
                                 const std::filesystem::path& file,
                                 const ErrorLocation& location,
                                 std::string_view extraMsg) {
    printLevel(code);
    fmt::print(" in ");
    fmt::print(fg(color::forest_green), "'{}'", file.string());
    fmt::print(" ");
    printLocation(location);
    fmt::print("\n");
    printCode(code);
    if (!extraMsg.empty()) {
      fmt::print(fg(color::sky_blue), "\n\t{}", extraMsg);
    }
    fmt::print("\n");
  }

  void ColoredStdoutSink::printCode(Code code) const { fmt::print("\t{}", prettyMessage(code)); }
  void ColoredStdoutSink::printLocation(const ErrorLocation& location) const { std::visit(*this, location); }

  ColoredStdoutSink& getColoredStdoutSink() {
    static ColoredStdoutSink COLORED_STDOUT_SINK;
    return COLORED_STDOUT_SINK;
  }
}  // namespace fluir::diagnostic
