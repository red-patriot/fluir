#ifndef FLUIR_COMPILER_TYPES_SYMBOL_TABLE_HPP
#define FLUIR_COMPILER_TYPES_SYMBOL_TABLE_HPP

#include <string>
#include <unordered_set>

#include "compiler/models/operator.hpp"
#include "compiler/types/type.hpp"

namespace fluir {
  /** A Hierarchical table of the available symbols at a specific scope */ s class SymbolTable {
   public:
    SymbolTable const* getParent() const;

    void addType(Type t);
    Type* getType(std::string_view name);
    Type const* getType(std::string_view name) const;

    void addOperator(Operator op, Type* returnType, Type* param);
    void addOperator(Operator op, Type* returnType, Type* param1, Type* param2);

   private:
  };
}  // namespace fluir

#endif
