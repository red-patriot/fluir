#ifndef FLUIR_COMPILER_TYPES_OPERATOR_HPP
#define FLUIR_COMPILER_TYPES_OPERATOR_HPP

#include <array>
#include <functional>

#include "compiler/models/operator.hpp"
#include "compiler/types/type.hpp"

namespace fluir::types {
  /** Represents the definition of an operator */
  class OperatorDefinition {
   public:
    OperatorDefinition(types::TypePtr parameter1,
                       ::fluir::Operator op,
                       types::TypePtr parameter2,
                       types::TypePtr returnType);
    OperatorDefinition(::fluir::Operator op, types::TypePtr parameter1, types::TypePtr returnType);

    ::fluir::Operator getOperator() const { return op_; }

    TypePtr getReturn() const { return returnType_; }
    std::array<TypePtr, 2> getParameters() const;

    friend bool operator==(const OperatorDefinition&, const OperatorDefinition&) = default;

   private:
    ::fluir::Operator op_; /**< The operator being applied */
    TypePtr returnType_;   /**< The type of the return value */
    TypePtr parameter1_;   /**< The type of the first parameter */
    TypePtr parameter2_;   /**< The type of the second parameter */
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
