#include "compiler/types/operator.hpp"

#include <cstdint>

namespace fluir::types {
  Operator::Operator(::fluir::Operator op, Type const* returnType, Type const* parameter1, Type const* parameter2) :
    op_(op), returnType_(returnType), parameter1_(parameter1), parameter2_(parameter2) { }

  Operator::Operator(::fluir::Operator op, Type const* returnType, Type const* parameter1) :
    Operator(op, returnType, parameter1, nullptr) { }

  std::array<Type const*, 2> Operator::getParameters() const { return {parameter1_, parameter2_}; }
}  // namespace fluir::types

namespace std {
  size_t hash<fluir::types::Operator>::operator()(fluir::types::Operator const& op) const noexcept {
    size_t seed = 0;
    seed ^= static_cast<uint8_t>(op.getOperator());
    constexpr hash<::fluir::types::Type const*> ptrHash{};
    seed ^= ptrHash(op.getReturn());
    const auto params = op.getParameters();
    seed ^= ptrHash(params.front());
    if (params.back()) {
      seed ^= ptrHash(params.back());
    }

    return seed;
  }

}  // namespace std
