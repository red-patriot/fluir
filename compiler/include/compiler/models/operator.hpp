#ifndef FLUIR_COMPILER_MODELS_OPERATOR_HPP
#define FLUIR_COMPILER_MODELS_OPERATOR_HPP

#include <string>

namespace fluir {
  enum class Operator {
    UNKNOWN,  //
    PLUS,     // +
    MINUS,    // -
    STAR,     // *
    SLASH     // /
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
      default:
        return "<UNKNOWN>";
    }
  }
};  // namespace fluir

#endif
