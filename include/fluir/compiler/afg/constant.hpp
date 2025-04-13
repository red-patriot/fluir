#ifndef FLUIR_COMPILER_AFG_CONSTANT_HPP
#define FLUIR_COMPILER_AFG_CONSTANT_HPP

#include "fluir/compiler/afg/node.hpp"

namespace fluir::afg {
  using Value = double;  // TODO: Support other types here

  class Constant : public Node {
   public:
    Type type() const { return type_; }
    template <Type t>
    auto& as() const {
      if constexpr (t == Type::DOUBLE_FP) {
        return as_;
      } else {
        throw TypeError{};
      }
    }

    bool equals(const Node& other) const override;

   private:
    Type type_{Type::UNKNOWN};
    Value as_;

    Constant(Type, Value);

    friend Constant makeDoubleConstant(double value);
  };

  Constant makeDoubleConstant(double value);
}  // namespace fluir::afg

#endif
