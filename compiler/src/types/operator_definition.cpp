#include <cstdint>

#include "compiler/types/operator_def.hpp"

namespace fluir::types {
  OperatorDefinition::OperatorDefinition(TypePtr parameter1,
                                         ::fluir::Operator op,
                                         TypePtr parameter2,
                                         TypePtr returnType) :
    op_(op), returnType_(returnType), parameter1_(parameter1), parameter2_(parameter2) { }

  OperatorDefinition::OperatorDefinition(::fluir::Operator op, TypePtr parameter1, TypePtr returnType) :
    OperatorDefinition(parameter1, op, nullptr /*UNARY*/, returnType) { }

  std::array<TypePtr, 2> OperatorDefinition::getParameters() const { return {parameter1_, parameter2_}; }

  bool CompareOperatorDefByParameters::operator()(OperatorDefinition const& lhs, OperatorDefinition const& rhs) const {
    return lhs.getOperator() == rhs.getOperator() && lhs.getParameters() == rhs.getParameters();
  }

}  // namespace fluir::types

namespace std {
  size_t hash<fluir::types::OperatorDefinition>::operator()(fluir::types::OperatorDefinition const& op) const noexcept {
    size_t seed = 0;
    seed ^= static_cast<uint8_t>(op.getOperator());
    constexpr hash<::fluir::types::TypePtr> ptrHash{};
    const auto params = op.getParameters();
    seed ^= ptrHash(params.front());
    if (params.back()) {
      seed ^= ptrHash(params.back());
    }

    return seed;
  }

}  // namespace std
