#ifndef FLUIR_COMPILER_AST_BLOCK_HPP
#define FLUIR_COMPILER_AST_BLOCK_HPP

namespace fluir::ast {
  struct Block {
    // TODO: Members
   private:
    friend bool operator==(const Block&, const Block&) = default;
  };

  constexpr Block EMPTY_BLOCK{};
}  // namespace fluir::ast

#endif
