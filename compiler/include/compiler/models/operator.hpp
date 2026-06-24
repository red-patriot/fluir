#ifndef FLUIR_COMPILER_MODELS_OPERATOR_HPP
#define FLUIR_COMPILER_MODELS_OPERATOR_HPP

#include <string>

namespace fluir {
  enum class Operator {
    UNKNOWN,        //
    PLUS,           // +
    MINUS,          // -
    STAR,           // *
    SLASH,          // /
    PLUS_PLUS,      // ++
    MINUS_MINUS,    // --
    EQUAL_EQUAL,    // ==
    BANG_EQUAL,     // !=
    GREATER,        // >
    LESS,           // <
    GREATER_EQUAL,  // >=
    LESS_EQUAL,     // <=
    BANG,           // !
    AND_AND,        // &&
    BAR_BAR,        // ||
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
      case Operator::EQUAL_EQUAL:
        return "==";
      case Operator::BANG_EQUAL:
        return "!=";
      case Operator::GREATER:
        return ">";
      case Operator::LESS:
        return "<";
      case Operator::GREATER_EQUAL:
        return ">=";
      case Operator::LESS_EQUAL:
        return "<=";
      case Operator::BANG:
        return "!";
      case Operator::AND_AND:
        return "&&";
      case Operator::BAR_BAR:
        return "||";
      case Operator::UNKNOWN:
        return "<UNKNOWN>";
    }
    return "<ERROR>";
  }
};  // namespace fluir

#endif
