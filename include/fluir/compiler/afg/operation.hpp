#ifndef FLUIR_COMPILER_AFG_OPERATION_HPP
#define FLUIR_COMPILER_AFG_OPERATION_HPP

#include "fluir/compiler/afg/node.hpp"
#include "fluir/compiler/models/operator.hpp"

namespace fluir::afg {
  class BinaryOp : public Node {
   public:
    BinaryOp(fluir::Operator op_, SharedNode lhs, SharedNode rhs);

    Type type() const override;

    bool equals(const Node& other) const override;

    fluir::Operator op;
    SharedNode lhs;
    SharedNode rhs;
  };
}  // namespace fluir::afg

#endif
