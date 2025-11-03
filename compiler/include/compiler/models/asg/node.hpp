#ifndef FLUIR_COMPILER_MODELS_ASG_NODE_HPP
#define FLUIR_COMPILER_MODELS_ASG_NODE_HPP

#include <memory>
#include <variant>
#include <vector>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"

namespace fluir::asg {
  struct NodeBase {
    ID id;
    FlowGraphLocation location;
  };

  struct Node;

  using SharedDependency = std::shared_ptr<Node>;
  using UniqueNode = std::unique_ptr<Node>;

  struct ConstantFP : public NodeBase {
    double value;
  };

  struct BinaryOp : public NodeBase {
    Operator op;
    SharedDependency lhs;
    SharedDependency rhs;
  };

  struct UnaryOp : public NodeBase {
    Operator op;
    SharedDependency operand;
  };

  struct Node : public std::variant<ConstantFP, BinaryOp, UnaryOp> {
    using variant::variant;

    template <typename Concrete>
    bool is() const {
      return std::holds_alternative<Concrete>(*this);
    }

    template <typename Concrete>
    Concrete& as() {
      return std::get<Concrete>(*this);
    }

    template <typename Concrete>
    const Concrete& as() const {
      return std::get<Concrete>(*this);
    }

    ID id() const {
      return std::visit([](const auto& n) { return n.id; }, *this);
    }
  };

  using DataFlowGraph = std::vector<UniqueNode>;

}  // namespace fluir::asg

#endif
