#ifndef FLUIR_COMPILER_PARSE_TREE_DECLARATION_HPP
#define FLUIR_COMPILER_PARSE_TREE_DECLARATION_HPP

#include <memory>
#include <unordered_map>
#include <variant>

#include "fluir/compiler/parse_tree/function_declaration.hpp"

namespace fluir::parse_tree {
  using Declaration = std::variant<FunctionDecl>;

  using Declarations = std::unordered_map<id_t, Declaration>;

}  // namespace fluir::parse_tree

#endif
