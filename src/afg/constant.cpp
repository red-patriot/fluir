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

}  // namespace fluir::afg
