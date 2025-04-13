#ifndef FLUIR_COMPILER_PARSE_TREE_AST_HPP
#define FLUIR_COMPILER_PARSE_TREE_AST_HPP

#include "fluir/compiler/parse_tree/declaration.hpp"

namespace fluir::parse_tree {
  struct ParseTree {
    Declarations declarations;
    friend bool operator==(const ParseTree&, const ParseTree&) = default;
  };
}  // namespace fluir::parse_tree

#endif
