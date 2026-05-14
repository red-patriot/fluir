#include "vm/code/string.hpp"

#include <cstring>

namespace fluir {
  String::String(const String& other) : size{other.size}, chars{std::make_unique<char[]>(size)} {
    std::memcpy(chars.get(), other.chars.get(), size);
  }
  String& String::operator=(const String& other) {
    size = other.size;
    chars = std::make_unique<char[]>(other.size);
    std::memcpy(chars.get(), other.chars.get(), size);
    return *this;
  }

  String createStaticString(const std::string_view in) {
    String out{in.length(), std::make_unique<char[]>(in.size())};
    std::memcpy(out.chars.get(), in.data(), in.size());
    return out;
  }
}  // namespace fluir
