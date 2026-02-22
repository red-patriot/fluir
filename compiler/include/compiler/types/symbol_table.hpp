#ifndef FLUIR_COMPILER_TYPES_SYMBOL_TABLE_HPP
#define FLUIR_COMPILER_TYPES_SYMBOL_TABLE_HPP

#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "compiler/models/id.hpp"
#include "compiler/models/operator.hpp"
#include "compiler/types/conversion.hpp"
#include "compiler/types/function_type.hpp"
#include "compiler/types/operator_def.hpp"
#include "compiler/types/type.hpp"
#include "compiler/types/typeid.hpp"

namespace fluir::types {
  /** A hierarchical table of the available symbols at a specific scope */
  class SymbolTable {
    //? Should all the type operations also follow scoping rules, or should they be global?
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

    /** Creates a new local scope for locals */
    void pushScope();
    /** Pops the top scope on the stack. This function has no effect if there are no scopes pushed */
    void popScope();

    /** Adds a local variable with the given ID and its type to the current scope */
    bool addLocalVariable(ID id, TypeID type);
    /** Retrieves the type of the local variable with the given ID */
    TypeID getLocalVariableType(ID id) const;

    /** Adds a function type to the table indexed by name.
     *  Returns a pointer to the stored signature, or nullptr if the name is already registered. */
    FunctionType const* addFunction(std::string name, FunctionType func);
    /** Returns the function signature for the given name, or nullptr if not registered. */
    FunctionType const* getFunctionType(const std::string& name) const;
    /** Returns the TypeID assigned to the named function's signature, or ID_INVALID if not registered */
    TypeID getFunctionTypeID(const std::string& name) const;

   private:
    using OverloadSet =
      std::unordered_set<OperatorDefinition, std::hash<OperatorDefinition>, CompareOperatorDefByParameters>;

    struct Scope {
      std::unordered_map<ID, TypeID> variables{};
    };

    /** Registers a function signature and returns its TypeID, deduplicating by structure */
    TypeID registerFunctionType(const FunctionType& func);

    TypeID nextTypeID_{static_cast<TypeID>(1)};  // 0 == ID_INVALID; unified counter for all types

    std::stack<Scope> localScopes_{};
    std::unordered_map<TypeID, Type> types_{};
    std::unordered_map<std::string, TypeID> typeNames_{};
    std::unordered_map<::fluir::Operator, OverloadSet> operators_{};
    std::unordered_map<TypeID, std::unordered_set<Conversion>> conversions_{};

    std::unordered_map<TypeID, FunctionType> functionTypes_{};
    std::unordered_map<FunctionType, TypeID> functionTypeIDs_{};
    std::unordered_map<std::string, TypeID> functionNames_{};
  };
}  // namespace fluir::types

#endif
