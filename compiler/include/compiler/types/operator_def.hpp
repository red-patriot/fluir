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
    OperatorDefinition(Type const* parameter1, ::fluir::Operator op, Type const* parameter2, Type const* returnType);
    OperatorDefinition(::fluir::Operator op, Type const* parameter1, Type const* returnType);

    ::fluir::Operator getOperator() const { return op_; }

    Type const* getReturn() const { return returnType_; }
    std::array<Type const*, 2> getParameters() const;

    friend bool operator==(const OperatorDefinition&, const OperatorDefinition&) = default;

   private:
    ::fluir::Operator op_;   /**< The operator being applied */
    Type const* returnType_; /**< The type of the return value */
    Type const* parameter1_; /**< The type of the first parameter */
    Type const* parameter2_; /**< The type of the second parameter */
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
