#ifndef FLUIR_COMPILER_AST_BLOCK_HPP
#define FLUIR_COMPILER_AST_BLOCK_HPP

#include "fluir/compiler/ast/constant.hpp"

namespace fluir::ast {
  struct Block {
    // TODO: Support multiple nodes
    Constant node;

   private:
    friend bool operator==(const Block&, const Block&) = default;
  };

  constexpr Block EMPTY_BLOCK{};
}  // namespace fluir::ast

#endif
