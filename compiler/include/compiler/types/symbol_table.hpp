#ifndef FLUIR_COMPILER_TYPES_SYMBOL_TABLE_HPP
#define FLUIR_COMPILER_TYPES_SYMBOL_TABLE_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "compiler/models/operator.hpp"
#include "compiler/types/operator.hpp"
#include "compiler/types/type.hpp"

namespace fluir::types {
  /** A hierarchical table of the available symbols at a specific scope */
  class SymbolTable {
   public:
    /** Add a known type to the symbol table */
    void addType(Type t);
    Type* getType(const std::string& name);
    Type const* getType(const std::string& name) const;

    void addOperator(Operator op);
    /** Get all known overloads of a given operator */
    std::vector<Operator const*> getOperatorOverloads(::fluir::Operator op) const;

   private:
    std::unordered_map<std::string, Type> types_;
    std::unordered_map<::fluir::Operator, std::unordered_set<Operator>> operators_;
  };
}  // namespace fluir::types

#endif
