#include "editor/core/literal_text.hpp"

#include <unordered_map>
#include <unordered_set>

#include <re2/re2.h>

namespace fluir::editor {
  namespace {
    const std::unordered_map<std::string_view, std::string_view> CONSTANTS{{"pi", "\u03C0"},
                                                                           {"e", "\u212F"},
                                                                           {"ln(2)", "\u33D12"},
                                                                           {"ln(10)", "\u33D110"},
                                                                           {"sqrt(2)", "\u221A2"},
                                                                           {"sqrt(3)", "\u221A3"},
                                                                           {"inf", "\u221E"}};

    bool isNamedConstant(std::string_view text) { return CONSTANTS.contains(text); }
    std::string getConstantSymbol(std::string_view text) { return std::string{CONSTANTS.at(text)}; }
  }  // namespace

  std::string renderLiteral(const pt::Constant& value) {
    if (value.typeTag != pt::Constant::F64 || !isNamedConstant(value.value)) {
      return value.value;
    }
    return getConstantSymbol(value.value);
  }

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
