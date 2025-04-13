#include "fluir/compiler/afg/constant.hpp"

#include <utility>

namespace fluir::afg {
  Constant::Constant(Type t, Value v) :
      type_(t),
      as_(std::move(v)) { }

  Constant makeDoubleConstant(double value) {
    return Constant{
        Type::DOUBLE_FP,
        value};
  }

  bool Constant::equals(const Node& other) const {
    auto& c = dynamic_cast<const Constant&>(other);
    return this->type_ == c.type_ &&
           this->as_ == c.as_;
  }

}  // namespace fluir::afg
