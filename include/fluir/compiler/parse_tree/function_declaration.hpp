#ifndef FLUIR_COMPILER_PARSE_TREE_FUNCTION_DECLARATION_HPP
#define FLUIR_COMPILER_PARSE_TREE_FUNCTION_DECLARATION_HPP

#include <string>

#include "fluir/compiler/parse_tree/block.hpp"
#include "fluir/compiler/parse_tree/utility.hpp"

namespace fluir::parse_tree {
  struct FunctionDecl {
    std::string name;
    fluir::id_t id;
    Block body;
    LocationInfo location;

   private:
    friend bool operator==(const FunctionDecl&, const FunctionDecl&) = default;
  };
}  // namespace fluir::parse_tree

#endif
