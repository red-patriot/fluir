#ifndef FLUIR_COMPILER_PARSE_TREE_CONSTANT_HPP
#define FLUIR_COMPILER_PARSE_TREE_CONSTANT_HPP

#include <variant>

#include "fluir/compiler/parse_tree/utility.hpp"

namespace fluir::parse_tree {
  using FpDouble = double;

  using Value = std::variant<FpDouble>;

  struct Constant {
    Value value;
    id_t id;
    LocationInfo location;

   private:
    friend bool operator==(const Constant&, const Constant&) = default;
  };
}  // namespace fluir::parse_tree

#endif
