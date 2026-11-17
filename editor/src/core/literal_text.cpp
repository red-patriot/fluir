#include "editor/core/literal_text.hpp"

#include <unordered_set>

#include <re2/re2.h>

namespace fluir::editor {
  namespace {
    bool isNamedConstant(std::string_view text) {
      static const std::unordered_set<std::string_view> NAMES{
        "pi", "e", "ln(2)", "ln(10)", "sqrt(2)", "sqrt(3)", "inf"};
      return NAMES.contains(text);
    }
  }  // namespace

  std::string renderLiteral(const pt::Constant& value) { return value.value; }

  std::string_view literalTypeName(const pt::Constant& value) { return ::fluir::pt::toString(value.typeTag); }

  bool isEditableLiteral(const pt::Constant& value) { return value.typeTag != pt::Constant::BOOL; }

  bool isValid(pt::Constant::Type type, std::string_view text) {
    static const RE2 intRe{R"(-?\d+)"};
    static const RE2 uintRe{R"(\d+)"};
    static const RE2 floatRe{R"(-?\d+(\.\d*)?)"};
    switch (type) {
      case pt::Constant::BOOL:
        return text == "true" || text == "false";
      case pt::Constant::F64:
        return RE2::FullMatch(text, floatRe) || isNamedConstant(text);
      case pt::Constant::I8:
      case pt::Constant::I16:
      case pt::Constant::I32:
      case pt::Constant::I64:
        return RE2::FullMatch(text, intRe);
      case pt::Constant::U8:
      case pt::Constant::U16:
      case pt::Constant::U32:
      case pt::Constant::U64:
        return RE2::FullMatch(text, uintRe);
    }
    return false;
  }
}  // namespace fluir::editor
