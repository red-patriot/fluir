#ifndef FLUIR_COMPILER_MODELS_ASG_NODE_HPP
#define FLUIR_COMPILER_MODELS_ASG_NODE_HPP

#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "compiler/models/id.hpp"
#include "compiler/models/literal_types.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
#include "compiler/types/operator_def.hpp"
#include "compiler/types/typeid.hpp"

namespace fluir::asg {
  enum class NodeKind {
    Constant,
    BinaryOperator,
    UnaryOperator,
  };

  class Node {
   public:
    virtual ~Node() = default;

    template <typename Concrete>
    [[nodiscard]] bool is() const {
      return Concrete::classOf(*this);
    }

    template <typename Concrete>
    Concrete* as() {
      return is<Concrete>() ? dynamic_cast<Concrete*>(this) : nullptr;
    }

    template <typename Concrete>
    Concrete const* as() const {
      return is<Concrete>() ? dynamic_cast<Concrete const*>(this) : nullptr;
    }

    [[nodiscard]] ID id() const { return id_; }
    [[nodiscard]] FlowGraphLocation location() const { return location_; }
    [[nodiscard]] NodeKind kind() const { return kind_; }
    [[nodiscard]] types::TypeID type() const { return type_; }

    void setType(types::TypeID type) { type_ = type; }

   protected:
    Node(const NodeKind kind, const ID id, const FlowGraphLocation& location) :
      kind_(kind), id_(id), location_(location) { }

   private:
    NodeKind kind_;
    ID id_;
    FlowGraphLocation location_;
    types::TypeID type_ = types::ID_INVALID;
  };

  using SharedDependency = std::shared_ptr<Node>;
  using UniqueNode = std::unique_ptr<Node>;

  class Constant : public Node {
   public:
    static bool classOf(const Node& node) { return node.kind() == NodeKind::Constant; }

    Constant(literals_types::Literal value, ID id, const FlowGraphLocation& location) :
      Node(NodeKind::Constant, id, location), value_(value) {
      setType(determineType(value_));
    }

    [[nodiscard]] const literals_types::Literal& value() const { return value_; }
    [[nodiscard]] const literals_types::F64& f64() const { return std::get<literals_types::F64>(value_); }
    [[nodiscard]] const literals_types::I8& i8() const { return std::get<literals_types::I8>(value_); }
    [[nodiscard]] const literals_types::I16& i16() const { return std::get<literals_types::I16>(value_); }
    [[nodiscard]] const literals_types::I32& i32() const { return std::get<literals_types::I32>(value_); }
    [[nodiscard]] const literals_types::I64& i64() const { return std::get<literals_types::I64>(value_); }
    [[nodiscard]] const literals_types::U8& u8() const { return std::get<literals_types::U8>(value_); }
    [[nodiscard]] const literals_types::U16& u16() const { return std::get<literals_types::U16>(value_); }
    [[nodiscard]] const literals_types::U32& u32() const { return std::get<literals_types::U32>(value_); }
    [[nodiscard]] const literals_types::U64& u64() const { return std::get<literals_types::U64>(value_); }

   private:
    literals_types::Literal value_;

    static types::TypeID determineType(literals_types::Literal value) {
      switch (value.index()) {
        case 0:
          return types::ID_F64;
        case 1:
          return types::ID_I8;
        case 2:
          return types::ID_I16;
        case 3:
          return types::ID_I32;
        case 4:
          return types::ID_I64;
        case 5:
          return types::ID_U8;
        case 6:
          return types::ID_U16;
        case 7:
          return types::ID_U32;
        case 8:
          return types::ID_U64;
        default:
          return types::ID_INVALID;
      }
    }
  };

  class BinaryOp : public Node {
   public:
    static bool classOf(const Node& node) { return node.kind() == NodeKind::BinaryOperator; }

    BinaryOp(
      const Operator op, SharedDependency lhs, SharedDependency rhs, const ID id, const FlowGraphLocation& location) :
      Node(NodeKind::BinaryOperator, id, location), op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) { }

    [[nodiscard]] const Operator& op() const { return op_; }
    [[nodiscard]] const SharedDependency& lhs() const { return lhs_; }
    [[nodiscard]] const SharedDependency& rhs() const { return rhs_; }
    [[nodiscard]] types::OperatorDefinition const* definition() const { return def_; }
    void setDefinition(types::OperatorDefinition const* def) {
      def_ = def;
      setType(def->getReturn());
    }

   private:
    Operator op_;
    SharedDependency lhs_;
    SharedDependency rhs_;
    types::OperatorDefinition const* def_ = nullptr;
  };

  class UnaryOp : public Node {
   public:
    static bool classOf(const Node& node) { return node.kind() == NodeKind::UnaryOperator; }

    UnaryOp(const Operator op, SharedDependency operand, const ID id, const FlowGraphLocation& location) :
      Node(NodeKind::UnaryOperator, id, location), op_(op), operand_(std::move(operand)) { }

    [[nodiscard]] const Operator& op() const { return op_; }
    [[nodiscard]] const SharedDependency& operand() const { return operand_; }
    [[nodiscard]] types::OperatorDefinition const* definition() const { return def_; }
    void setDefinition(types::OperatorDefinition const* def) {
      def_ = def;
      setType(def->getReturn());
    }

   private:
    Operator op_;
    SharedDependency operand_;
    types::OperatorDefinition const* def_ = nullptr;
  };

  using DataFlowGraph = std::vector<UniqueNode>;

}  // namespace fluir::asg

#endif
