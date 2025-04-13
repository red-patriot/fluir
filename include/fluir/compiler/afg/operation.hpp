#ifndef FLUIR_COMPILER_AFG_OPERATION_HPP
#define FLUIR_COMPILER_AFG_OPERATION_HPP

#include "fluir/compiler/afg/dependency.hpp"
#include "fluir/compiler/afg/node.hpp"
#include "fluir/compiler/models/operator.hpp"

namespace fluir::afg {
  class UnaryOp : public Node {
   public:
    UnaryOp(fluir::Operator op_, const Dependency& operand_);

    Type type() const override;

    bool equals(const Node& other) const override;

    fluir::Operator op;
    Dependency operand;
  };

  class BinaryOp : public Node {
   public:
    BinaryOp(fluir::Operator op_, const Dependency& lhs, const Dependency& rhs);

    Type type() const override;

    bool equals(const Node& other) const override;

    fluir::Operator op;
    Dependency lhs;
    Dependency rhs;
  };
}  // namespace fluir::afg

#endif
