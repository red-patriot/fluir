#include "compiler/utility/diagnostic/pretty_msg.hpp"

#include <unordered_map>
#include <utility>

#include <fmt/format.h>

namespace fluir::diagnostic {
  namespace {
    using enum Code;

    const std::unordered_map<Code, std::string> messages = {{GENERIC_ERROR, "An unknown error was emitted"},
                                                            {GENERIC_WARNING, "An unknown warning was emitted"},
                                                            {GENERIC_NOTE, ""}};

  }  // namespace

  std::string prettyMessage(Code code) {
    if (messages.contains(code)) {
      return messages.at(code);
    }

    // If there isn't a known message, just print out the number
    return fmt::format("ERROR 0x{:X}", std::to_underlying(code));
  }
}  // namespace fluir::diagnostic
