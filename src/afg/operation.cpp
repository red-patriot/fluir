#include "fluir/compiler/afg/operation.hpp"

namespace fluir::afg {
  UnaryOp::UnaryOp(fluir::Operator op_, const Dependency& operand_) :
      op(op_),
      operand(operand_) { }

  Type UnaryOp::type() const {
    return Type::DOUBLE_FP;
  }

  bool UnaryOp::equals(const Node& other) const {
    auto& concrete = dynamic_cast<const UnaryOp&>(other);
    return this->op == concrete.op &&
           this->operand->equals(*concrete.operand);
  }

  BinaryOp::BinaryOp(fluir::Operator op_, const Dependency& lhs_, const Dependency& rhs_) :
      op(op_),
      lhs(lhs_),
      rhs(rhs_) { }

  Type BinaryOp::type() const {
    return Type::DOUBLE_FP;
  }

  bool BinaryOp::equals(const Node& other) const {
    auto& concrete = dynamic_cast<const BinaryOp&>(other);
    return this->op == concrete.op &&
           this->lhs->equals(*concrete.lhs) &&
           this->rhs->equals(*concrete.rhs);
  }
};  // namespace fluir::afg
