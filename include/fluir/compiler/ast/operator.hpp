#ifndef FLUIR_COMPILER_AST_OPERATOR_HPP
#define FLUIR_COMPILER_AST_OPERATOR_HPP

#include "fluir/compiler/ast/utility.hpp"

namespace fluir::ast {
  enum class Operator {
    UNKNOWN,  //
    PLUS,     // +
    MINUS,    // -
    STAR,     // *
    SLASH     // /
  };

  struct Binary {
    fluir::id_t id;
    fluir::id_t lhs;
    fluir::id_t rhs;
    Operator op;

    LocationInfo location;

   private:
    friend bool operator==(const Binary&, const Binary&) = default;
  };
}  // namespace fluir::ast

#endif
