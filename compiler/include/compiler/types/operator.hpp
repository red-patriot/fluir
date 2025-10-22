#ifndef FLUIR_COMPILER_TYPES_OPERATOR_HPP
#define FLUIR_COMPILER_TYPES_OPERATOR_HPP

#include <array>
#include <functional>

#include "compiler/models/operator.hpp"
#include "compiler/types/type.hpp"

namespace fluir::types {
  /** Represents the definition of an operator */
  class Operator {
   public:
    Operator(::fluir::Operator op, Type* returnType, Type* parameter1, Type* parameter2);
    Operator(::fluir::Operator op, Type* returnType, Type* parameter1);

    ::fluir::Operator getOperator() const { return op_; }

    Type const* getReturn() const { return returnType_; }
    std::array<Type const*, 2> getParameters() const;

    friend bool operator==(const Operator&, const Operator&) = default;

   private:
    ::fluir::Operator op_; /**< The operator being applied */
    Type* returnType_;     /**< The type of the return value */
    Type* parameter1_;     /**< The type of the first parameter */
    Type* parameter2_;     /**< The type of the second parameter */
  };
}  // namespace fluir::types

namespace std {
  template <>
  struct hash<::fluir::types::Operator> {
    size_t operator()(::fluir::types::Operator const& op) const noexcept;
  };
}  // namespace std

#endif
