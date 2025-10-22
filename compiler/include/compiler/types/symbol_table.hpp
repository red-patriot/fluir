#ifndef FLUIR_COMPILER_TYPES_SYMBOL_TABLE_HPP
#define FLUIR_COMPILER_TYPES_SYMBOL_TABLE_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "compiler/models/operator.hpp"
#include "compiler/types/conversion.hpp"
#include "compiler/types/operator.hpp"
#include "compiler/types/type.hpp"

namespace fluir::types {
  /** A hierarchical table of the available symbols at a specific scope */
  class SymbolTable {
   public:
    /** Add a known type to the symbol table, returns a pointer to the type */
    Type const* addType(Type t);
    /** Gets a type */
    Type const* getType(const std::string& name) const;

    /** Add an operator to the table */
    Operator const* addOperator(Operator op);
    /** Get all known overloads of a given operator */
    std::vector<Operator const*> getOperatorOverloads(::fluir::Operator op) const;

    /** Adds an implicit conversion operation  */
    void addCast(Type const* from, Type const* to);
    /** Adds an explicit conversion operation */
    void addConversion(Type const* from, Type const* to);
    /** Tests if `from` can be implicitly converted to a `to`  */
    bool canCast(Type const* from, Type const* to);
    /** Tests if `from` can be explicitly converted to a `to` */
    bool canConvert(Type const* from, Type const* to);

   private:
    std::unordered_map<std::string, Type> types_;
    std::unordered_map<::fluir::Operator, std::unordered_set<Operator>> operators_;
    std::unordered_map<Type const*, std::unordered_set<Conversion>> conversions_;
  };
}  // namespace fluir::types

#endif
