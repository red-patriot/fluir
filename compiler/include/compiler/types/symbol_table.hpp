#ifndef FLUIR_COMPILER_TYPES_SYMBOL_TABLE_HPP
#define FLUIR_COMPILER_TYPES_SYMBOL_TABLE_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "compiler/models/operator.hpp"
#include "compiler/types/conversion.hpp"
#include "compiler/types/operator_def.hpp"
#include "compiler/types/type.hpp"
#include "compiler/types/typeid.hpp"

namespace fluir::types {
  /** A hierarchical table of the available symbols at a specific scope */
  class SymbolTable {
   public:
    SymbolTable();

    /** Add a known type to the symbol table, returns a pointer to the type */
    TypeID addType(Type t);
    /** Gets a type */
    TypeID getTypeID(const std::string& name) const;
    Type const* getType(TypeID id) const;

    /** Add an operator to the table */
    OperatorDefinition const* addOperator(OperatorDefinition op);
    /** Get all known overloads of a given operator */
    std::vector<OperatorDefinition const*> getOperatorOverloads(Operator op) const;
    /** Perform operator overload resolution for the operator with the given operands */
    OperatorDefinition const* selectOverload(TypeID lhs, Operator op, TypeID rhs);
    /** Perform operator overload resolution for the operator with the given operands */
    OperatorDefinition const* selectOverload(Operator op, TypeID operand);

    /** Adds an implicit conversion operation  */
    void addImplicitConversion(TypeID from, TypeID to);
    /** Adds an explicit conversion operation */
    void addExplicitConversion(TypeID from, TypeID to);
    /** Tests if `from` can be implicitly converted to a `to`  */
    bool canImplicitlyConvert(TypeID from, TypeID to);
    /** Tests if `from` can be explicitly converted to a `to` */
    bool canExplicitlyConvert(TypeID from, TypeID to);

   private:
    using OverloadSet =
      std::unordered_set<OperatorDefinition, std::hash<OperatorDefinition>, CompareOperatorDefByParameters>;
    std::unordered_map<TypeID, Type> types_{};
    std::unordered_map<std::string, TypeID> typeNames_{};
    std::unordered_map<::fluir::Operator, OverloadSet> operators_{};
    std::unordered_map<TypeID, std::unordered_set<Conversion>> conversions_{};
  };
}  // namespace fluir::types

#endif
