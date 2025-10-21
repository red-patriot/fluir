#ifndef FLUIR_COMPILER_TYPES_OPERATOR_HPP
#define FLUIR_COMPILER_TYPES_OPERATOR_HPP

#include <set>

#include "compiler/models/operator.hpp"
#include "compiler/types/type.hpp"

namespace fluir {
  /** Represents the definition of an operator */
  class OperatorDef {
   public:
    Operator getOperator() const { return op_; }

    Type const* getReturn() const { return returnType_; }
    std::multiset<Type const*> getParameters() const;

   private:
    Operator op_;      /**< The operator being applied */
    Type* returnType_; /**< The type of the return value */
    Type* parameter1_; /**< The type of the first parameter */
    Type* parameter2_; /**< The type of the second parameter */
  };
}  // namespace fluir

#endif
