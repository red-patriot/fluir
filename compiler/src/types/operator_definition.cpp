#include <cstdint>

#include "compiler/types/operator_def.hpp"

namespace fluir::types {
  OperatorDefinition::OperatorDefinition(Type const* parameter1,
                                         ::fluir::Operator op,
                                         Type const* parameter2,
                                         Type const* returnType) :
    op_(op), returnType_(returnType), parameter1_(parameter1), parameter2_(parameter2) { }

  OperatorDefinition::OperatorDefinition(::fluir::Operator op, Type const* parameter1, Type const* returnType) :
    OperatorDefinition(parameter1, op, nullptr, returnType) { }

  std::array<Type const*, 2> OperatorDefinition::getParameters() const { return {parameter1_, parameter2_}; }
}  // namespace fluir::types

namespace std {
  size_t hash<fluir::types::OperatorDefinition>::operator()(fluir::types::OperatorDefinition const& op) const noexcept {
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
