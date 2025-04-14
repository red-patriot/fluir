#include "fluir/compiler/afg/constant.hpp"

#include <utility>

namespace fluir::afg {
  DoubleConstant::DoubleConstant(double value_, LocationInfo location) :
      Node(std::move(location)),
      value(value_) { }

  DoubleConstant makeDoubleConstant(double value, LocationInfo location) {
    return DoubleConstant{value, location};
  }

  bool DoubleConstant::equals(const Node& other) const {
    auto& concrete = dynamic_cast<const DoubleConstant&>(other);
    return this->value == concrete.value;
  }

}  // namespace fluir::afg
