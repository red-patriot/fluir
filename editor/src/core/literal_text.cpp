#include "editor/core/literal_text.hpp"

#include <array>
#include <charconv>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <type_traits>
#include <variant>

#include <fmt/format.h>

#include "compiler/frontend/text_parse.hpp"

namespace fluir::editor {
  std::string renderLiteral(const pt::Constant& value) { return value.value; }

  std::string_view literalTypeName(const pt::Constant& value) { return ::fluir::pt::toString(value.typeTag); }

  bool isEditableLiteral(const pt::Constant& value) { return value.typeTag != pt::Constant::BOOL; }

}  // namespace fluir::editor
