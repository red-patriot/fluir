module;

#include <memory>
#include <variant>
#include <vector>

export module fluir.models.asg.node;
export import fluir.models.id;
export import fluir.models.location;
export import fluir.models.operators;

namespace fluir::asg {
  export struct NodeBase {
    ID id;
    FlowGraphLocation location;
  };

  export struct Node;

  export using SharedDependency = std::shared_ptr<Node>;

  export struct ConstantFP : public NodeBase {
    double value;
  };

  export struct BinaryOp : public NodeBase {
    Operator op;
    SharedDependency lhs;
    SharedDependency rhs;
  };

  export struct UnaryOp : public NodeBase {
    Operator op;
    SharedDependency operand;
  };

  export struct Node : public std::variant<ConstantFP, BinaryOp, UnaryOp> {
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

  export using DataFlowGraph = std::vector<Node>;

}  // namespace fluir::asg
