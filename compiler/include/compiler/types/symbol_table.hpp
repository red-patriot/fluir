#ifndef FLUIR_COMPILER_TYPES_SYMBOL_TABLE_HPP
#define FLUIR_COMPILER_TYPES_SYMBOL_TABLE_HPP

#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "compiler/models/id.hpp"
#include "compiler/models/operator.hpp"
#include "compiler/types/conversion.hpp"
#include "compiler/types/function_type.hpp"
#include "compiler/types/operator_def.hpp"
#include "compiler/types/type.hpp"

namespace fluir::types {
  /** A hierarchical table of the available symbols at a specific scope */
  class SymbolTable {
   public:
    SymbolTable();

    /** Add an operator to the table */
    OperatorDefinition const* addOperator(OperatorDefinition op);
    /** Get all known overloads of a given operator */
    std::vector<OperatorDefinition const*> getOperatorOverloads(Operator op) const;
    /** Perform operator overload resolution for the operator with the given operands */
    OperatorDefinition const* selectOverload(Type const* lhs, Operator op, Type const* rhs);
    /** Perform operator overload resolution for the operator with the given operands */
    OperatorDefinition const* selectOverload(Operator op, Type const* operand);

    /** Adds an implicit conversion operation  */
    void addImplicitConversion(Type const* from, Type const* to);
    /** Adds an explicit conversion operation */
    void addExplicitConversion(Type const* from, Type const* to);
    /** Tests if `from` can be implicitly converted to a `to`  */
    bool canImplicitlyConvert(Type const* from, Type const* to);
    /** Tests if `from` can be explicitly converted to a `to` */
    bool canExplicitlyConvert(Type const* from, Type const* to);

    /** Creates a new local scope for locals */
    void pushScope();
    /** Pops the top scope on the stack. This function has no effect if there are no scopes pushed */
    void popScope();

    /** Adds a local variable with the given ID and its type to the current scope */
    bool addLocalVariable(ID id, Type const* type);
    /** Retrieves the type of the local variable with the given ID */
    Type const* getLocalVariableType(ID id) const;

    /** Adds a function type to the table indexed by name.
     *  Returns the TypeID assigned to the function, or ID_INVALID if the name is already registered. */
    FunctionType const* addFunction(std::string name, FunctionType func);
    /** Returns the function signature for the given name, or nullptr if not registered. */
    FunctionType const* getFunctionType(const std::string& name) const;
    /** Determines if the given function is a builtin with special rules around type checking
     * TODO: This will be removed/refactored once generics are implemented
     */
    bool isMagicBuiltin(const FunctionType* func);

   private:
    using OverloadSet =
      std::unordered_set<OperatorDefinition, std::hash<OperatorDefinition>, CompareOperatorDefByParameters>;

    struct Scope {
      std::unordered_map<ID, Type const*> variables{};
    };

    /** Registers a function signature and returns its TypeID, deduplicating by structure */
    FunctionType const* registerFunctionType(const FunctionType& func);

    std::stack<Scope> localScopes_{};
    std::unordered_map<::fluir::Operator, OverloadSet> operators_{};
    std::unordered_map<Type const*, std::unordered_set<Conversion>> conversions_{};

    struct SignatureHash {
      using is_transparent = void;
      size_t operator()(const FunctionType& func) const;
      size_t operator()(FunctionType const* func) const { return operator()(*func); }
      size_t operator()(const std::unique_ptr<FunctionType>& func) const { return operator()(func.get()); }
    };
    struct SignatureEq {
      using is_transparent = void;
      bool operator()(FunctionType const* lhs, FunctionType const* rhs) const;
      bool operator()(const std::unique_ptr<FunctionType>& lhs, const std::unique_ptr<FunctionType>& rhs) const {
        return operator()(lhs.get(), rhs.get());
      }
      bool operator()(const std::unique_ptr<FunctionType>& lhs, FunctionType const* rhs) const {
        return operator()(lhs.get(), rhs);
      }
      bool operator()(FunctionType const* lhs, const std::unique_ptr<FunctionType>& rhs) const {
        return operator()(lhs, rhs.get());
      }
    };

    std::unordered_set<std::unique_ptr<FunctionType>, SignatureHash, SignatureEq> functionTypes_{};
    std::unordered_map<std::string, FunctionType const*> functionNames_{};
  };
}  // namespace fluir::types

#endif
