#ifndef FLUIR_COMPILER_FRONTEND_PARSE_TREE_PARSE_TREE_HPP
#define FLUIR_COMPILER_FRONTEND_PARSE_TREE_PARSE_TREE_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "bytecode/version.hpp"
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

  struct Call {
    struct Argument {
      std::string name;
      int index;
    };
    using Arguments = std::vector<Argument>;
    struct Return { };

    ID id;
    FlowGraphLocation location;

    std::string target;
    std::optional<Return> _return;  // For now, function calls have only one return max
                                    // TODO: Support multiple return values
    Arguments arguments;
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

  using Node = std::variant<Binary, Unary, Constant, Call>;
  struct Block {
    using Nodes = std::unordered_map<ID, Node>;
    using Conduits = std::unordered_map<ID, Conduit>;

    Nodes nodes;
    Conduits conduits;

    friend bool operator==(const Block&, const Block&) = default;
  };

  inline const Block EMPTY_BLOCK = {};

  struct FunctionDecl {
    struct Parameter {
      ID id;
      int index;

      std::string name;
      std::string typeName;

      friend bool operator==(const Parameter&, const Parameter&) = default;
    };

    struct Return {
      ID id;

      std::string typeName;

      friend bool operator==(const Return&, const Return&) = default;
    };

    struct InputBlock {
      std::vector<Parameter> parameters;

      friend bool operator==(const InputBlock&, const InputBlock&) = default;
    };

    struct OutputBlock {
      std::optional<Return> ret;

      friend bool operator==(const OutputBlock&, const OutputBlock&) = default;
    };

    ID id;
    FlowGraphLocation location;

    std::string name;
    Block body;
    std::optional<InputBlock> input;
    std::optional<OutputBlock> output;

    friend bool operator==(const FunctionDecl&, const FunctionDecl&) = default;
  };

  using Declaration = std::variant<FunctionDecl>;  // TODO: Support other top-level declarations here

  struct Header {
    Version version{.major = 0, .minor = 0, .patch = 0};

    friend bool operator==(const Header&, const Header&) = default;
  };

  struct ParseTree {
    Header header{};
    std::unordered_map<ID, Declaration> declarations;

    friend bool operator==(const ParseTree&, const ParseTree&) = default;
  };
}  // namespace fluir::pt

#endif
