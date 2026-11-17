#include "editor/core/literal_text.hpp"

#include <re2/re2.h>

namespace fluir::editor {
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
        return RE2::FullMatch(text, floatRe);
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
