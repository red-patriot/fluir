#pragma once

#include <string_view>

namespace fluir::editor {

  /** True for a non-empty ASCII `[A-Za-z_][A-Za-z0-9_]*`. */
  bool isValidIdentifier(std::string_view name);

}  // namespace fluir::editor
