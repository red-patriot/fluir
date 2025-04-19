#ifndef FLUIR_COMPILER_FRONTEND_PARSE_TREE_PARSE_TREE_HPP
#define FLUIR_COMPILER_FRONTEND_PARSE_TREE_PARSE_TREE_HPP

#include <string>
#include <unordered_map>
#include <variant>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"

namespace fluir::pt {
  using Float = double;
  using Literal = Float;  // TODO: Support other literal types

  struct Constant {
    FlowGraphLocation location;
    Literal value;

    friend bool operator==(const Constant&, const Constant&) = default;
  };

  struct Binary {
    FlowGraphLocation location;
    ID lhs;
    ID rhs;
    fluir::Operator op;

    friend bool operator==(const Binary&, const Binary&) = default;
  };

  struct Unary {
    FlowGraphLocation location;
    ID lhs;
    fluir::Operator op;

    friend bool operator==(const Unary&, const Unary&) = default;
  };

  using Node = std::variant<Binary, Unary, Constant>;
  using Block = std::unordered_map<ID, Node>;

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
