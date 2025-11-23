#ifndef FLUIR_COMPILER_TYPES_OPERATOR_HPP
#define FLUIR_COMPILER_TYPES_OPERATOR_HPP

#include <array>
#include <functional>

#include "compiler/models/operator.hpp"
#include "compiler/types/typeid.hpp"

namespace fluir::types {
  /** Represents the definition of an operator */
  class OperatorDefinition {
   public:
    OperatorDefinition(TypeID parameter1, ::fluir::Operator op, TypeID parameter2, TypeID returnType);
    OperatorDefinition(::fluir::Operator op, TypeID parameter1, TypeID returnType);

    ::fluir::Operator getOperator() const { return op_; }

    TypeID getReturn() const { return returnType_; }
    std::array<TypeID, 2> getParameters() const;

    friend bool operator==(const OperatorDefinition&, const OperatorDefinition&) = default;

   private:
    ::fluir::Operator op_; /**< The operator being applied */
    TypeID returnType_;    /**< The type of the return value */
    TypeID parameter1_;    /**< The type of the first parameter */
    TypeID parameter2_;    /**< The type of the second parameter */
  };

  struct CompareOperatorDefByParameters {
    bool operator()(OperatorDefinition const& lhs, OperatorDefinition const& rhs) const;
  };
}  // namespace fluir::types

namespace std {
  template <>
  struct hash<::fluir::types::OperatorDefinition> {
    size_t operator()(::fluir::types::OperatorDefinition const& op) const noexcept;
  };
}  // namespace std

#endif
