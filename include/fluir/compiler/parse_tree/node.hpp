#ifndef FLUIR_COMPILER_PARSE_TREE_NODE_HPP
#define FLUIR_COMPILER_PARSE_TREE_NODE_HPP

#include <unordered_map>
#include <variant>

#include "fluir/compiler/parse_tree/constant.hpp"
#include "fluir/compiler/parse_tree/operator.hpp"

namespace fluir::parse_tree {
  using Node = std::variant<Constant,
                            Binary,
                            Unary>;

  using Nodes = std::unordered_map<fluir::id_t, Node>;
}  // namespace fluir::parse_tree

#endif
