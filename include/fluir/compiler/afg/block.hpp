#ifndef FLUIR_COMPILER_AFG_BLOCK_HPP
#define FLUIR_COMPILER_AFG_BLOCK_HPP

#include <concepts>
#include <memory>
#include <vector>

namespace fluir {
  enum class Operator {
    UNKNOWN,
    PLUS,
    MINUS,
    STAR,
    SLASH,
  };

  namespace afg {
    class Node { };

    using SharedNode = std::shared_ptr<Node>;
    template <typename ConcreteNode>
      requires std::derived_from<ConcreteNode, Node>
    SharedNode shared(ConcreteNode node);

    class Block {
     public:
      std::vector<SharedNode> pipelines;
    };

    class BinOp : public Node {
     public:
      Operator op;
      SharedNode lhs;
      SharedNode rhs;
    };

    class Constant : public Node {
     public:
      Type type;
      Value value;
    };

  }  // namespace afg
}  // namespace fluir

#endif
