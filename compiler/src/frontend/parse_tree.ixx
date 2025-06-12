module;

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"

export module fluir.frontend.parse_tree;

namespace fluir::pt {
  export using Float = double;
  export using Literal = Float;  // TODO: Support other literal types

  export struct Constant {
    ID id;
    FlowGraphLocation location;
    Literal value;

    friend bool operator==(const Constant&, const Constant&) = default;
  };

  export struct Binary {
    ID id;
    FlowGraphLocation location;
    ID lhs = 0;
    ID rhs = 0;
    fluir::Operator op;

    friend bool operator==(const Binary&, const Binary&) = default;
  };

  export struct Unary {
    ID id;
    FlowGraphLocation location;
    ID lhs = 0;
    fluir::Operator op;

    friend bool operator==(const Unary&, const Unary&) = default;
  };

  export struct Conduit {
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

  export using Node = std::variant<Binary, Unary, Constant>;
  export struct Block {
    using Nodes = std::unordered_map<ID, Node>;
    using Conduits = std::unordered_map<ID, Conduit>;

    Nodes nodes;
    Conduits conduits;

    friend bool operator==(const Block&, const Block&) = default;
  };

  export inline const Block EMPTY_BLOCK = {};

  export struct FunctionDecl {
    ID id;
    FlowGraphLocation location;

    std::string name;
    Block body;

    friend bool operator==(const FunctionDecl&, const FunctionDecl&) = default;
  };

  export using Declaration = std::variant<FunctionDecl>;  // TODO: Support other top-level declarations here

  export struct ParseTree {
    std::unordered_map<ID, Declaration> declarations;

    friend bool operator==(const ParseTree&, const ParseTree&) = default;
  };
}  // namespace fluir::pt
