#ifndef FLUIR_COMPILER_AST_CONSTANT_HPP
#define FLUIR_COMPILER_AST_CONSTANT_HPP

#include <variant>

#include "fluir/compiler/ast/utility.hpp"

namespace fluir::ast {
  using FpDouble = double;

  using Value = std::variant<FpDouble>;

  struct Constant {
    Value value;
    id_t id;
    LocationInfo location;

   private:
    friend bool operator==(const Constant&, const Constant&) = default;
  };
}  // namespace fluir::ast

#endif
