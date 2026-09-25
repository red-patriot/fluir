#ifndef FLUIR_COMPILER_FRONTEND_PARSE_TREE_PARSE_TREE_HPP
#define FLUIR_COMPILER_FRONTEND_PARSE_TREE_PARSE_TREE_HPP

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <indirect.h>

#include "bytecode/version.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/literal_types.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
#include "compiler/utility/macros.hpp"

namespace fluir::pt {
  using namespace literals_types;

  /** Optional annotations a consumer hangs on the tree. */
  struct NoAnnotations {
    struct Comment {
      friend bool operator==(const Comment&, const Comment&) = default;
    };
    struct Constant {
      friend bool operator==(const Constant&, const Constant&) = default;
    };
    struct Binary {
      friend bool operator==(const Binary&, const Binary&) = default;
    };
    struct Unary {
      friend bool operator==(const Unary&, const Unary&) = default;
    };
    struct Call {
      friend bool operator==(const Call&, const Call&) = default;
    };
    struct Conditional {
      friend bool operator==(const Conditional&, const Conditional&) = default;
    };
    struct FunctionDecl {
      friend bool operator==(const FunctionDecl&, const FunctionDecl&) = default;
    };
    struct ParseTree {
      friend bool operator==(const ParseTree&, const ParseTree&) = default;
    };
  };

  template <class A>
  struct CommentT {
    ID id;
    FlowGraphLocation location;
    std::string text;

    FLUIR_NO_UNIQUE_ADDRESS typename A::Comment annotation{};

    friend bool operator==(const CommentT& lhs, const CommentT& rhs) = default;
  };

  template <class A>
  struct ConstantT {
    ID id;
    FlowGraphLocation location;
    Literal value;

    FLUIR_NO_UNIQUE_ADDRESS typename A::Constant annotation{};

    friend bool operator==(const ConstantT&, const ConstantT&) = default;
  };

  template <class A>
  struct BinaryT {
    ID id;
    FlowGraphLocation location;
    ID lhs = 0;
    ID rhs = 0;
    fluir::Operator op;

    FLUIR_NO_UNIQUE_ADDRESS typename A::Binary annotation{};

    friend bool operator==(const BinaryT&, const BinaryT&) = default;
  };

  template <class A>
  struct UnaryT {
    ID id;
    FlowGraphLocation location;
    ID lhs = 0;
    fluir::Operator op;

    FLUIR_NO_UNIQUE_ADDRESS typename A::Unary annotation{};

    friend bool operator==(const UnaryT&, const UnaryT&) = default;
  };

  struct CallArgument {
    std::string name;
    int index;

    friend bool operator==(const CallArgument&, const CallArgument&) = default;
  };

  struct CallReturn {
    friend bool operator==(const CallReturn&, const CallReturn&) = default;
  };

  template <class A>
  struct CallT {
    using Argument = CallArgument;
    using Arguments = std::vector<CallArgument>;
    using Return = CallReturn;

    ID id;
    FlowGraphLocation location;

    std::string target;
    std::optional<CallReturn> _return;  // For now, function calls have only one return max
                                        // TODO: Support multiple return values
    Arguments arguments;

    FLUIR_NO_UNIQUE_ADDRESS typename A::Call annotation{};

    friend bool operator==(const CallT&, const CallT&) = default;
  };

  template <class A>
  struct BlockT;

  struct BlockPort {
    ID outerId; /**< Outward-facing ID to bridge data to the outer scope */
    ID innerId; /**< Inward-facing ID to bridge dataflow to the inner scope */
    int y;      /**< Position on the vertical wall where this port lives */

    friend bool operator==(const BlockPort&, const BlockPort&) = default;
  };

  using BlockPorts = std::unordered_map<ID, BlockPort>;

  template <class A>
  struct ConditionalT {
    ID id;
    FlowGraphLocation location;

    BlockPort condition;
    BlockPorts inputs;
    BlockPorts outputs;

    xyz::indirect<BlockT<A>> thenScope;
    xyz::indirect<BlockT<A>> elseScope;

    FLUIR_NO_UNIQUE_ADDRESS typename A::Conditional annotation{};

    friend bool operator==(const ConditionalT&, const ConditionalT&) = default;
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

  template <class A>
  using NodeT = std::variant<BinaryT<A>, UnaryT<A>, ConstantT<A>, CallT<A>, CommentT<A>, ConditionalT<A>>;

  template <class A>
  struct BlockT {
    //! This should probably be deprecated in favor of Scope above
    using Nodes = std::unordered_map<ID, NodeT<A>>;
    using Conduits = std::unordered_map<ID, Conduit>;

    Nodes nodes;
    Conduits conduits;

    friend bool operator==(const BlockT&, const BlockT&) = default;
  };

  struct FunctionParameter {
    ID id;
    int index;

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

  template <class A>
  struct FunctionDeclT {
    using Parameter = FunctionParameter;
    using Return = FunctionReturn;
    using InputBlock = FunctionInputBlock;
    using OutputBlock = FunctionOutputBlock;

    ID id;
    FlowGraphLocation location;

    std::string name;
    BlockT<A> body;
    std::optional<FunctionInputBlock> input;
    std::optional<FunctionOutputBlock> output;

    FLUIR_NO_UNIQUE_ADDRESS typename A::FunctionDecl annotation{};

    friend bool operator==(const FunctionDeclT&, const FunctionDeclT&) = default;
  };

  // TODO: Support other top-level declarations here
  template <class A>
  using DeclarationT = std::variant<FunctionDeclT<A>, CommentT<A>>;

  struct Header {
    Version version{.major = 0, .minor = 0, .patch = 0};

    friend bool operator==(const Header&, const Header&) = default;
  };

  template <class A>
  struct ParseTreeT {
    Header header{};
    std::unordered_map<ID, DeclarationT<A>> declarations;

    FLUIR_NO_UNIQUE_ADDRESS typename A::ParseTree annotation{};

    friend bool operator==(const ParseTreeT&, const ParseTreeT&) = default;
  };

  using Comment = CommentT<NoAnnotations>;
  using Constant = ConstantT<NoAnnotations>;
  using Binary = BinaryT<NoAnnotations>;
  using Unary = UnaryT<NoAnnotations>;
  using Call = CallT<NoAnnotations>;
  using Conditional = ConditionalT<NoAnnotations>;
  using Node = NodeT<NoAnnotations>;
  using Block = BlockT<NoAnnotations>;
  using FunctionDecl = FunctionDeclT<NoAnnotations>;
  using Declaration = DeclarationT<NoAnnotations>;
  using ParseTree = ParseTreeT<NoAnnotations>;

  inline const Block EMPTY_BLOCK = {};

}  // namespace fluir::pt

#endif
