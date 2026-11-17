#ifndef FLUIR_EDITOR_CORE_LITERAL_TEXT_HPP
#define FLUIR_EDITOR_CORE_LITERAL_TEXT_HPP

#include <optional>
#include <string>
#include <string_view>

#include "compiler/frontend/parse_tree/parse_tree.hpp"

namespace fluir::editor {

  /** Formats `value` as text, matching how it is drawn as a node label. */
  std::string renderLiteral(const pt::Constant& value);

  /** `value`'s source type tag. */
  std::string_view literalTypeName(const pt::Constant& value);

  /** True for the 8 integral alternatives and F64. Bool is click to toggle. */
  bool isEditableLiteral(const pt::Constant& value);

}  // namespace fluir::editor

#endif
