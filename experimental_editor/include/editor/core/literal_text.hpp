#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "editor/core/tree.hpp"

namespace fluir::editor {

  /** Formats `value` as text, matching how it is drawn as a node label. */
  std::string renderLiteral(const et::Literal& value);

  /** `value`'s source type tag. */
  std::string_view literalTypeName(const et::Literal& value);

  /** True for the 8 integral alternatives (I8..U64) and F64; bool is not editable. */
  bool isEditableLiteral(const et::Literal& value);

  /** Parses `text` as the alternative `like` already holds, so the variant index
   *  never changes. nullopt on an empty/non-numeric/out-of-range (or non-finite
   *  F64) draft, or a non-editable `like`. */
  std::optional<et::Literal> tryParseLiteral(const et::Literal& like, std::string_view text);

}  // namespace fluir::editor
