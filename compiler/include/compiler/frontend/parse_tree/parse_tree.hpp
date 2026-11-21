#ifndef FLUIR_COMPILER_FRONTEND_PARSE_TREE_PARSE_TREE_HPP
#define FLUIR_COMPILER_FRONTEND_PARSE_TREE_PARSE_TREE_HPP

#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <indirect.h>

#include "bytecode/version.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"

namespace fluir::pt {
  struct Comment {
    ID id;
    FlowGraphLocation location;
    std::string text;

    friend bool operator==(const Comment& lhs, const Comment& rhs) = default;
  };

  struct Constant {
    /** The type a literal constant can have */
    enum Type {
      F64,
      I8,
      I16,
      I32,
      I64,
      U8,
      U16,
      U32,
      U64,
      BOOL,
    };
    ID id;
    FlowGraphLocation location;
    std::string value;
    Type typeTag;

    friend bool operator==(const Constant&, const Constant&) = default;
  };

  inline std::string_view toString(Constant::Type t) {
    switch (t) {
      case Constant::F64:
        return "F64";
      case Constant::I8:
        return "I8";
      case Constant::I16:
        return "I16";
      case Constant::I32:
        return "I32";
      case Constant::I64:
        return "I64";
      case Constant::U8:
        return "U8";
      case Constant::U16:
        return "U16";
      case Constant::U32:
        return "U32";
      case Constant::U64:
        return "U64";
      case Constant::BOOL:
        return "BOOL";
    }
    return "<UNKNOWN TYPE>";
  }

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

  struct CallArgument {
    std::string name;
    int index;

    friend bool operator==(const CallArgument&, const CallArgument&) = default;
  };

  struct CallReturn {
    friend bool operator==(const CallReturn&, const CallReturn&) = default;
  };

  struct Call {
    using Argument = CallArgument;
    using Arguments = std::vector<CallArgument>;
    using Return = CallReturn;

    ID id;
    FlowGraphLocation location;

    std::string target;
    std::optional<CallReturn> _return;  // For now, function calls have only one return max
                                        // TODO: Support multiple return values
    Arguments arguments;

    friend bool operator==(const Call&, const Call&) = default;
  };

  struct Block;

  struct BlockPort {
    ID innerId; /**< Inward-facing ID to bridge dataflow to the inner scope */
    int y;      /**< Position on the vertical wall where this port lives */

    friend bool operator==(const BlockPort&, const BlockPort&) = default;
  };

  using BlockPorts = std::vector<BlockPort>;

  struct Conditional {
    ID id;
    FlowGraphLocation location;

    BlockPort condition;
    BlockPorts inputs;
    BlockPorts outputs;

    xyz::indirect<Block> thenScope;
    xyz::indirect<Block> elseScope;

    friend bool operator==(const Conditional&, const Conditional&) = default;
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

  using Node = std::variant<Binary, Unary, Constant, Call, Comment, Conditional>;

  struct Block {
    //! This should probably be deprecated in favor of Scope above
    using Nodes = std::unordered_map<ID, Node>;
    using Conduits = std::unordered_map<ID, Conduit>;

    Nodes nodes;
    Conduits conduits;

    friend bool operator==(const Block&, const Block&) = default;
  };

  struct FunctionParameter {
    ID id;

    std::string name;
    std::string typeName;

    friend bool operator==(const FunctionParameter&, const FunctionParameter&) = default;
  };

  struct FunctionReturn {
    ID id;

    std::string typeName;

    friend bool operator==(const FunctionReturn&, const FunctionReturn&) = default;
  };

  struct FunctionInputBlock {
    std::vector<FunctionParameter> parameters;

    friend bool operator==(const FunctionInputBlock&, const FunctionInputBlock&) = default;
  };

  struct FunctionOutputBlock {
    std::optional<FunctionReturn> ret;

    friend bool operator==(const FunctionOutputBlock&, const FunctionOutputBlock&) = default;
  };

  struct FunctionDecl {
    using Parameter = FunctionParameter;
    using Return = FunctionReturn;
    using InputBlock = FunctionInputBlock;
    using OutputBlock = FunctionOutputBlock;

    ID id;
    FlowGraphLocation location;

    std::string name;
    Block body;
    std::optional<FunctionInputBlock> input;
    std::optional<FunctionOutputBlock> output;

    friend bool operator==(const FunctionDecl&, const FunctionDecl&) = default;
  };

  // TODO: Support other top-level declarations here
  using Declaration = std::variant<FunctionDecl, Comment>;

  struct Header {
    Version version{.major = 0, .minor = 0, .patch = 0};

    friend bool operator==(const Header&, const Header&) = default;
  };

  struct ParseTree {
    Header header{};
    std::unordered_map<ID, Declaration> declarations;

    friend bool operator==(const ParseTree&, const ParseTree&) = default;
  };

  inline const Block EMPTY_BLOCK = {};

}  // namespace fluir::pt

#endif
