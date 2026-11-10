#include "editor/core/choices.hpp"

namespace fluir::editor {

  const std::vector<std::string>& binaryOperators() {
    static const std::vector<std::string> operators{"+", "-", "*", "/", "==", "!=", ">=", "<=", ">", "<", "&&", "||"};
    return operators;
  }

  const std::vector<std::string>& unaryOperators() {
    static const std::vector<std::string> operators{"+", "-", "++", "--", "!"};
    return operators;
  }

  const std::vector<std::string>& builtinTypes() {
    static const std::vector<std::string> types{"F64", "I8", "I16", "I32", "I64", "U8", "U16", "U32", "U64", "BOOL"};
    return types;
  }

  std::optional<fluir::Operator> operatorNamed(std::string_view text) {
    for (int i = static_cast<int>(fluir::Operator::PLUS); i <= static_cast<int>(fluir::Operator::BAR_BAR); ++i) {
      if (stringify(static_cast<fluir::Operator>(i)) == text) {
        return static_cast<fluir::Operator>(i);
      }
    }
    return std::nullopt;
  }

}  // namespace fluir::editor
