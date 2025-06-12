module;

#include <string>

export module fluir.models.operators;

namespace fluir {
  export enum class Operator {
    UNKNOWN,  //
    PLUS,     // +
    MINUS,    // -
    STAR,     // *
    SLASH     // /
  };

  export inline std::string_view stringify(Operator op) {
    switch (op) {
      case Operator::PLUS:
        return "+";
      case Operator::MINUS:
        return "-";
      case Operator::STAR:
        return "*";
      case Operator::SLASH:
        return "/";
      default:
        return "<UNKNOWN>";
    }
  }
};  // namespace fluir
