#ifndef FLUIR_EDITOR_CORE_CHOICES_HPP
#define FLUIR_EDITOR_CORE_CHOICES_HPP

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "compiler/models/operator.hpp"

namespace fluir::editor {

  /** The operators a binary node may take, spelled as in source. */
  const std::vector<std::string>& binaryOperators();

  /** The operators a unary node may take, spelled as in source. */
  const std::vector<std::string>& unaryOperators();

  /** The builtin type names a parameter or return may take. */
  const std::vector<std::string>& builtinTypes();

  /** The operator spelled `text`, if any. */
  std::optional<fluir::Operator> operatorNamed(std::string_view text);

}  // namespace fluir::editor

#endif
