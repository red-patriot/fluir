#ifndef FLUIR_COMPILER_FRONTEND_PARSE_TREE_PARSE_TREE_HPP
#define FLUIR_COMPILER_FRONTEND_PARSE_TREE_PARSE_TREE_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "compiler/models/id.hpp"
#include "compiler/models/literal_types.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"

namespace fluir::pt {
  using namespace literals_types;

  struct Constant {
    ID id;
    FlowGraphLocation location;
    Literal value;

    friend bool operator==(const Constant&, const Constant&) = default;
  };

  struct Binary {
    ID id;
    FlowGraphLocation location;
    ID lhs = 0;
    ID rhs = 0;
    fluir::Operator op;

    friend bool operator==(const Binary&, const Binary&) = default;
  };

  struct Unary {
    ID id;
    FlowGraphLocation location;
    ID lhs = 0;
    fluir::Operator op;

    friend bool operator==(const Unary&, const Unary&) = default;
  };

  struct Conduit {
    struct Output {
      ID target = INVALID_ID;
      int index = 0;
      friend bool operator==(const Output&, const Output&) = default;
    };
    // TODO: Support segment types

    ID id = INVALID_ID;
    ID input = INVALID_ID;
    int index = 0;
    std::vector<Output> children;

    friend bool operator==(const Conduit&, const Conduit&) = default;
  };

  using Node = std::variant<Binary, Unary, Constant>;
  struct Block {
    using Nodes = std::unordered_map<ID, Node>;
    using Conduits = std::unordered_map<ID, Conduit>;

    Nodes nodes;
    Conduits conduits;

    friend bool operator==(const Block&, const Block&) = default;
  };

  inline const Block EMPTY_BLOCK = {};

  struct FunctionDecl {
    ID id;
    FlowGraphLocation location;

    std::string name;
    Block body;

    friend bool operator==(const FunctionDecl&, const FunctionDecl&) = default;
  };

  using Declaration = std::variant<FunctionDecl>;  // TODO: Support other top-level declarations here

  struct ParseTree {
    std::unordered_map<ID, Declaration> declarations;

    friend bool operator==(const ParseTree&, const ParseTree&) = default;
  };
}  // namespace fluir::pt

#endif
