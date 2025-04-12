#ifndef FLUIR_COMPILER_AST_BLOCK_HPP
#define FLUIR_COMPILER_AST_BLOCK_HPP

#include "fluir/compiler/ast/node.hpp"

namespace fluir::ast {
  struct Block {
    // TODO: Support multiple nodes
    Nodes nodes;

   private:
    friend bool operator==(const Block&, const Block&) = default;
  };

  inline const Block EMPTY_BLOCK{.nodes = {}};
}  // namespace fluir::ast

#endif
