#ifndef FLUIR_COMPILER_FRONTEND_PARSE_TREE_PARSE_TREE_HPP
#define FLUIR_COMPILER_FRONTEND_PARSE_TREE_PARSE_TREE_HPP

#include <string>
#include <unordered_map>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"

namespace fluir::pt {
  struct Block {
    friend bool operator==(const Block&, const Block&) = default;
  };

  inline const Block EMPTY_BLOCK = {};

  struct FunctionDecl {
    FlowGraphLocation location;

    std::string name;
    Block body;

    friend bool operator==(const FunctionDecl&, const FunctionDecl&) = default;
  };

  using Declaration = FunctionDecl;  // TODO: Support other top-level declarations here

  struct ParseTree {
    std::unordered_map<ID, Declaration> declarations;

    friend bool operator==(const ParseTree&, const ParseTree&) = default;
  };
}  // namespace fluir::pt

#endif
