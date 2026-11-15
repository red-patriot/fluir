#include "editor/core/identifier.hpp"

#include <algorithm>

namespace fluir::editor {

  namespace {
    bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
    bool isAlnum(char c) { return isAlpha(c) || (c >= '0' && c <= '9'); }
  }  // namespace

  bool isValidIdentifier(std::string_view name) {
    return !name.empty() && isAlpha(name.front()) && std::ranges::all_of(name, isAlnum);
  }

}  // namespace fluir::editor
