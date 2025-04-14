#ifndef FLUIR_COMPILER_PARSE_TREE_OPERATOR_HPP
#define FLUIR_COMPILER_PARSE_TREE_OPERATOR_HPP

#include "fluir/compiler/models/operator.hpp"
#include "fluir/compiler/parse_tree/utility.hpp"

namespace fluir::parse_tree {
  using fluir::Operator;

  struct Unary {
    fluir::id_t id;
    fluir::id_t lhs;
    Operator op;

    LocationInfo location;

   private:
    friend bool operator==(const Unary&, const Unary&) = default;
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
}  // namespace fluir::parse_tree

#endif
