#include "fluir/compiler/afg/operation.hpp"

namespace fluir::afg {
  BinaryOp::BinaryOp(fluir::Operator op_, SharedNode lhs_, SharedNode rhs_) :
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
