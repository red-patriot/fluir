#ifndef FLUIR_VM_CODE_STRING_HPP
#define FLUIR_VM_CODE_STRING_HPP

#include <cstring>
#include <memory>
#include <string_view>

namespace fluir {
  struct String {
    size_t size{0};
    std::unique_ptr<char[]> chars{nullptr};

    String() = default;
    String(size_t s, std::unique_ptr<char[]> c) : size(s), chars(std::move(c)) { }
    String(const String&);
    String& operator=(const String&);
    String(String&&) = default;
    String& operator=(String&&) = default;
    ~String() = default;

    friend bool operator==(const String& lhs, const String& rhs) {
      return lhs.size == rhs.size && memcmp(lhs.chars.get(), rhs.chars.get(), lhs.size) == 0;
    }
  };

  String createStaticString(const std::string_view in);
}  // namespace fluir

#endif
