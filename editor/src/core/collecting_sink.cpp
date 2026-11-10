#include "editor/core/collecting_sink.hpp"

#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "compiler/utility/diagnostic/pretty_msg.hpp"

namespace fluir::editor {

  void CollectingSink::report(diagnostic::Code code,
                              const std::filesystem::path& file,
                              const Sink::ErrorLocation& location,
                              std::string_view extraMsg) {
    const std::string loc = std::visit(
      [](const auto& value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, int>) {
          return fmt::format("line {}", value);
        } else {
          return fmt::format("element {}", fmt::join(value, ":"));
        }
      },
      location);

    std::string message;
    if (!file.empty()) {
      message += file.string();
      message += ": ";
    }
    message += fmt::format("{}: {}", loc, diagnostic::prettyMessage(code));
    if (!extraMsg.empty()) {
      message += ' ';
      message.append(extraMsg);
    }
    messages_.push_back(std::move(message));
  }

}  // namespace fluir::editor
