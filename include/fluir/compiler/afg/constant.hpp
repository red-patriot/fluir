#ifndef FLUIR_COMPILER_AFG_CONSTANT_HPP
#define FLUIR_COMPILER_AFG_CONSTANT_HPP

#include "fluir/compiler/afg/node.hpp"

namespace fluir::afg {
  // TODO: Support other types here

  class DoubleConstant : public Node {
   public:
    explicit DoubleConstant(double value_);

    Type type() const { return Type::DOUBLE_FP; }
    double as() const {
      return value;
    }

    bool equals(const Node& other) const override;

    double value;
  };

  DoubleConstant makeDoubleConstant(double value);
}  // namespace fluir::afg

#endif
