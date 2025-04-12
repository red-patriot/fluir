#ifndef FLUIR_COMPILER_AST_NODE_HPP
#define FLUIR_COMPILER_AST_NODE_HPP

#include <unordered_map>
#include <variant>

#include "fluir/compiler/ast/constant.hpp"
#include "fluir/compiler/ast/operator.hpp"

namespace fluir::ast {
  using Node = std::variant<Constant,
                            Binary>;

  using Nodes = std::unordered_map<fluir::id_t, Node>;
}  // namespace fluir::ast

#endif
