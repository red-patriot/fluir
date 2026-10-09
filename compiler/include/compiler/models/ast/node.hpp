#ifndef FLUIR_COMPILER_MODELS_AST_NODE_HPP
#define FLUIR_COMPILER_MODELS_AST_NODE_HPP

#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include "compiler/models/id.hpp"
#include "compiler/models/literal_types.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
#include "compiler/types/operator_def.hpp"
#include "compiler/types/typeid.hpp"
#include "compiler/utility/context.hpp"

namespace fluir::ast {
  enum class NodeKind { Constant, BinaryOperator, UnaryOperator, Cast, LocalWrite, LocalRead, Call, Conditional };

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

    [[nodiscard]] const FullID& fullId() const { return id_; }
    [[nodiscard]] ID id() const { return id_.back(); }
    [[nodiscard]] FlowGraphLocation location() const { return location_; }
    [[nodiscard]] NodeKind kind() const { return kind_; }
    [[nodiscard]] types::TypeID type() const { return type_; }

    void setType(types::TypeID type) { type_ = type; }

   protected:
    Node(const NodeKind kind, FullID id, const FlowGraphLocation& location) :
      kind_(kind), id_(std::move(id)), location_(location) {
      assert(!id_.empty() && "Full ID of a node must have at least one element");
    }

   private:
    NodeKind kind_;
    FullID id_;
    FlowGraphLocation location_;
    types::TypeID type_ = types::ID_INVALID;
  };

  using UniqueNode = std::unique_ptr<Node>;
  using DataFlowGraph = std::vector<UniqueNode>;

  /** A dependency on another node in the graph */
  class Dependency {
   public:
    explicit Dependency(UniqueNode child, std::optional<unsigned> index = std::nullopt) :
      child_(std::move(child)), index_(index) { }

    [[nodiscard]] Node& get() { return *child_; }
    [[nodiscard]] const Node& get() const { return *child_; }
    [[nodiscard]] bool hasIndex() const noexcept { return index_.has_value(); }
    [[nodiscard]] unsigned index() const { return index_.value(); }
    [[nodiscard]] const FullID& fullId() const { return child_->fullId(); }
    [[nodiscard]] ID id() const { return child_->id(); }
    [[nodiscard]] FlowGraphLocation location() const { return child_->location(); }
    [[nodiscard]] NodeKind kind() const { return child_->kind(); }
    [[nodiscard]] types::TypeID type() const { return index_ ? type_ : child_->type(); }
    [[nodiscard]] types::TypeID fullType() const { return child_->type(); }

    void setType(types::TypeID t, const Context& ctx) {
      child_->setType(t);
      if (!index_) {
        return;
      }
      const auto* descriptor = ctx.symbolTable.getType(t);
      if (descriptor && descriptor->is<types::Product>()) {
        type_ = descriptor->as<types::Product>()->at(*index_);
      } else {
        type_ = t;
      }
    }

   private:
    UniqueNode child_;                      /**< The dependency node */
    types::TypeID type_{types::ID_INVALID}; /**< The type at `index_` of `child_`'s output if `index_` is set */
    std::optional<unsigned> index_;         /**< The index into `child_`'s outputs to read from */
  };

  struct ScopeInput {
    Dependency node;
    ID innerId;
  };

  template <typename NodeType, typename... Args>
  inline auto createNode(Args&&... args) {
    return std::make_unique<NodeType>(std::forward<Args>(args)...);
  }
  template <typename NodeType, typename... Args>
  inline Dependency createDependency(Args&&... args) {
    return Dependency{std::make_unique<NodeType>(std::forward<Args>(args)...), std::nullopt};
  }
  template <typename NodeType, typename... Args>
  inline Dependency createDependencyWithIndex(unsigned index, Args&&... args) {
    return Dependency{std::make_unique<NodeType>(std::forward<Args>(args)...), index};
  }
  inline Dependency createDependency(std::optional<unsigned> index, UniqueNode node) {
    return Dependency{std::move(node), index};
  }
  template <typename NodeType>
  inline auto clone(const std::unique_ptr<NodeType>& p) {
    return createNode<NodeType>(*p);
  }
  /** Deep copies a dependency of the given type */
  template <typename NodeType>
  inline Dependency clone(const Dependency& d) {
    const auto* concrete = d.get().as<NodeType>();
    assert(concrete && "clone called on a different node kind");
    return Dependency{createNode<NodeType>(*concrete), d.hasIndex() ? std::optional{d.index()} : std::nullopt};
  }

  class Constant : public Node {
   public:
    static bool classOf(const Node& node) { return node.kind() == NodeKind::Constant; }

    Constant(literals_types::Literal value, FullID id, const FlowGraphLocation& location) :
      Node(NodeKind::Constant, std::move(id), location), value_(value) {
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
    [[nodiscard]] const literals_types::BOOL& boolean() const { return std::get<literals_types::BOOL>(value_); }

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
        case 9:
          return types::ID_BOOL;
        default:
          return types::ID_INVALID;
      }
    }
  };

  class BinaryOp : public Node {
   public:
    static bool classOf(const Node& node) { return node.kind() == NodeKind::BinaryOperator; }

    BinaryOp(const Operator op, Dependency lhs, Dependency rhs, FullID id, const FlowGraphLocation& location) :
      Node(NodeKind::BinaryOperator, std::move(id), location), op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) { }

    [[nodiscard]] const Operator& op() const { return op_; }
    [[nodiscard]] Dependency& lhs() { return lhs_; }
    [[nodiscard]] const Dependency& lhs() const { return lhs_; }
    [[nodiscard]] Dependency& rhs() { return rhs_; }
    [[nodiscard]] const Dependency& rhs() const { return rhs_; }
    [[nodiscard]] types::OperatorDefinition const* definition() const { return def_; }
    void setDefinition(types::OperatorDefinition const* def) {
      def_ = def;
      setType(def->getReturn());
    }

   private:
    Operator op_;
    Dependency lhs_;
    Dependency rhs_;
    types::OperatorDefinition const* def_ = nullptr;
  };

  class UnaryOp : public Node {
   public:
    static bool classOf(const Node& node) { return node.kind() == NodeKind::UnaryOperator; }

    UnaryOp(const Operator op, Dependency operand, FullID id, const FlowGraphLocation& location) :
      Node(NodeKind::UnaryOperator, std::move(id), location), op_(op), operand_(std::move(operand)) { }

    [[nodiscard]] const Operator& op() const { return op_; }
    [[nodiscard]] const Dependency& operand() const { return operand_; }
    [[nodiscard]] Dependency& operand() { return operand_; }
    [[nodiscard]] types::OperatorDefinition const* definition() const { return def_; }
    void setDefinition(types::OperatorDefinition const* def) {
      def_ = def;
      setType(def->getReturn());
    }

   private:
    Operator op_;
    Dependency operand_;
    types::OperatorDefinition const* def_ = nullptr;
  };

  class Cast : public Node {
   public:
    static bool classOf(const Node& node) { return node.kind() == NodeKind::Cast; }

    Cast(types::TypeID to, Dependency operand, FullID id, const FlowGraphLocation& location) :
      Node(NodeKind::Cast, std::move(id), location), operand_(std::move(operand)) {
      setType(to);
    }

    [[nodiscard]] types::TypeID to() const { return type(); }
    [[nodiscard]] types::TypeID from() const { return operand_.type(); }
    [[nodiscard]] const Dependency& operand() const { return operand_; }
    [[nodiscard]] Dependency& operand() { return operand_; }

   private:
    Dependency operand_;
  };

  class Conditional : public Node {
   public:
    static bool classOf(const Node& node) { return node.kind() == NodeKind::Conditional; }

    Conditional(FullID id,
                const FlowGraphLocation& location,
                Dependency condition,
                std::vector<ScopeInput> inputs,
                std::vector<ID> outputs,
                DataFlowGraph thenBody,
                DataFlowGraph elseBody) :
      Node(NodeKind::Conditional, std::move(id), location),
      condition_(std::move(condition)),
      inputs_(std::move(inputs)),
      outputs_(std::move(outputs)),
      then_(std::move(thenBody)),
      else_(std::move(elseBody)) { }

    [[nodiscard]] const Dependency& condition() const { return condition_; }
    [[nodiscard]] Dependency& condition() { return condition_; }
    [[nodiscard]] const DataFlowGraph& thenBody() const { return then_; }
    [[nodiscard]] DataFlowGraph& thenBody() { return then_; }
    [[nodiscard]] const DataFlowGraph& elseBody() const { return else_; }
    [[nodiscard]] DataFlowGraph& elseBody() { return else_; }
    [[nodiscard]] const std::vector<ScopeInput>& inputs() const { return inputs_; }
    [[nodiscard]] std::vector<ScopeInput>& inputs() { return inputs_; }
    [[nodiscard]] const std::vector<ID>& outputs() const { return outputs_; }

   private:
    Dependency condition_;
    std::vector<ScopeInput> inputs_;
    std::vector<ID> outputs_;
    DataFlowGraph then_;
    DataFlowGraph else_;
  };

  class LocalWrite : public Node {
   public:
    static bool classOf(const Node& node) { return node.kind() == NodeKind::LocalWrite; }

    LocalWrite(FullID writeID, Dependency child, const FlowGraphLocation& location) :
      Node(NodeKind::LocalWrite, std::move(writeID), location), child_(std::move(child)) { }

    LocalWrite(Dependency child, const FlowGraphLocation& location) :
      Node(NodeKind::LocalWrite, child.fullId(), location), child_(std::move(child)) { }

    [[nodiscard]] const Dependency& child() const { return child_; }
    [[nodiscard]] Dependency& child() { return child_; }
    [[nodiscard]] ID variable() const { return id(); }

   private:
    Dependency child_;
  };

  class LocalRead : public Node {
   public:
    static bool classOf(const Node& node) { return node.kind() == NodeKind::LocalRead; }

    LocalRead(ID variable, FullID parentID, const FlowGraphLocation& location) :
      LocalRead(variable, 0, std::move(parentID), location) { }

    LocalRead(ID variable, int varIndex, FullID parentID, const FlowGraphLocation& location) :
      Node(NodeKind::LocalRead, std::move(parentID), location), variable_(variable), varIndex_(varIndex) { }

    [[nodiscard]] const FullID& parent() const { return fullId(); }
    /** Returns the ID of the read variable */
    [[nodiscard]] ID variable() const { return variable_; }
    /** Returns the index of the read variable */
    [[nodiscard]] int varIndex() const { return varIndex_; }

   private:
    ID variable_;
    int varIndex_;
  };

  class Call : public Node {
   public:
    static bool classOf(const Node& node) { return node.kind() == NodeKind::Call; }

    Call(std::string target, std::vector<Dependency> arguments, FullID id, const FlowGraphLocation& location) :
      Node(NodeKind::Call, std::move(id), location), target_(std::move(target)), arguments_(std::move(arguments)) { }

    const std::string& target() const { return target_; }
    const std::vector<Dependency>& arguments() const { return arguments_; }
    std::vector<Dependency>& arguments() { return arguments_; }

   private:
    std::string target_;                /**< The name of the target function to call */
    std::vector<Dependency> arguments_; /**< The arguments to pass to the function */
  };
}  // namespace fluir::ast

#endif
