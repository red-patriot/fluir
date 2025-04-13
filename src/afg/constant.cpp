#include "fluir/compiler/afg/constant.hpp"

#include <utility>

namespace fluir::afg {
  DoubleConstant::DoubleConstant(double value_) :
      value(value_) { }

  DoubleConstant makeDoubleConstant(double value) {
    return DoubleConstant{value};
  }

  bool DoubleConstant::equals(const Node& other) const {
    auto& concrete = dynamic_cast<const DoubleConstant&>(other);
    return this->value == concrete.value;
  }

}  // namespace fluir::afg
