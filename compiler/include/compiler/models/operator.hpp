#ifndef FLUIR_COMPILER_MODELS_OPERATOR_HPP
#define FLUIR_COMPILER_MODELS_OPERATOR_HPP

#include <string>

namespace fluir {
  enum class Operator {
    UNKNOWN,      //
    PLUS,         // +
    MINUS,        // -
    STAR,         // *
    SLASH,        // /
    PLUS_PLUS,    // ++
    MINUS_MINUS,  // --
  };

  inline std::string_view stringify(Operator op) {
    switch (op) {
      case Operator::PLUS:
        return "+";
      case Operator::MINUS:
        return "-";
      case Operator::STAR:
        return "*";
      case Operator::SLASH:
        return "/";
      case Operator::PLUS_PLUS:
        return "++";
      case Operator::MINUS_MINUS:
        return "--";
      default:
        return "<UNKNOWN>";
    }
  }
};  // namespace fluir

#endif
