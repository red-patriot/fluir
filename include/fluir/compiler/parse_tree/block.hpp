#ifndef FLUIR_COMPILER_PARSE_TREE_BLOCK_HPP
#define FLUIR_COMPILER_PARSE_TREE_BLOCK_HPP

#include "fluir/compiler/parse_tree/node.hpp"

namespace fluir::parse_tree {
  struct Block {
    // TODO: Support multiple nodes
    Nodes nodes;

   private:
    friend bool operator==(const Block&, const Block&) = default;
  };

  inline const Block EMPTY_BLOCK{.nodes = {}};
}  // namespace fluir::parse_tree

#endif
