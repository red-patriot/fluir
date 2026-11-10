#include "compiler/types/symbol_table.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>

namespace fluir::types {
  SymbolTable::SymbolTable() { }

  OperatorDefinition const* SymbolTable::addOperator(OperatorDefinition op) {
    auto [it, added] = operators_[op.getOperator()].insert(std::move(op));
    if (!added) {
      return *it == op ? &*it : nullptr;
    }
    return &*it;
  }
  std::vector<OperatorDefinition const*> SymbolTable::getOperatorOverloads(::fluir::Operator op) const {
    std::vector<OperatorDefinition const*> result;
    std::ranges::transform(operators_.at(op), std::back_inserter(result), [](auto const& elem) { return &elem; });
    return result;
  }

  OperatorDefinition const* SymbolTable::selectOverload(Type const* lhs, Operator op, Type const* rhs) {
    /* 1. Get all candidates
     * 2. Filter out all where at least one operand doesn't match
     * 3. Filter out all where there is no cast between given LHS and LHS
     * 4. Filter out all where there is no cast between given RHS and RHS
     */

    auto candidates = getOperatorOverloads(op);
    auto removed = std::ranges::remove_if(candidates, [&](OperatorDefinition const* candidate) {
      return candidate->getParameters()[0] != lhs && candidate->getParameters()[1] != rhs;
    });
    candidates.erase(removed.begin(), removed.end());
    if (candidates.empty()) {
      // Fail if no viable candidates are identified
      return nullptr;
    }

    removed = std::ranges::remove_if(candidates, [&](OperatorDefinition const* candidate) {
      return lhs != candidate->getParameters()[0] && !canImplicitlyConvert(lhs, candidate->getParameters()[0]);
    });
    candidates.erase(removed.begin(), removed.end());
    removed = std::ranges::remove_if(candidates, [&](OperatorDefinition const* candidate) {
      return rhs != candidate->getParameters()[1] && !canImplicitlyConvert(rhs, candidate->getParameters()[1]);
    });
    candidates.erase(removed.begin(), removed.end());

    if (candidates.size() != 1) {
      // The call is potentially ambiguous with casts. Try to find an exact match and prefer that if it exists
      auto exact = std::ranges::find_if(candidates, [&](OperatorDefinition const* candidate) {
        return candidate->getParameters()[0] == lhs && candidate->getParameters()[1] == rhs;
      });
      if (exact != candidates.end()) {
        return *exact;
      }

      // The call is ambiguous
      return nullptr;
    }
    return candidates.front();
  }

  OperatorDefinition const* SymbolTable::selectOverload(Operator op, Type const* operand) {
    /* 1. Get all candidates
     * 2. Filter out all binary operators
     * 3. Filter out all where there is no cast between given operand and operand
     */

    auto candidates = getOperatorOverloads(op);
    auto removed = std::ranges::remove_if(
      candidates, [](OperatorDefinition const* candidate) { return candidate->getParameters()[1]; });
    candidates.erase(removed.begin(), removed.end());
    if (candidates.empty()) {
      // Fail if no viable candidates are identified
      return nullptr;
    }
    removed = std::ranges::remove_if(candidates, [&](OperatorDefinition const* candidate) {
      return operand != candidate->getParameters()[0] && !canImplicitlyConvert(operand, candidate->getParameters()[0]);
    });
    candidates.erase(removed.begin(), removed.end());

    if (candidates.size() != 1) {
      // The call is potentially ambiguous with casts. Try to find an exact match and prefer that if it exists
      auto exact = std::ranges::find_if(
        candidates, [&](OperatorDefinition const* candidate) { return candidate->getParameters()[0] == operand; });
      if (exact != candidates.end()) {
        return *exact;
      }

      // The call is ambiguous
      return nullptr;
    }
    return candidates.front();
  }

  void SymbolTable::addImplicitConversion(Type const* from, Type const* to) {
    if (from == to) {
      return;
    }
    Conversion c{.from = from, .to = to, .isImplicit = true};
    conversions_[from].insert(c);
  }
  void SymbolTable::addExplicitConversion(Type const* from, Type const* to) {
    if (from == to) {
      return;
    }

    conversions_[from].insert(Conversion{.from = from, .to = to, .isImplicit = false});
  }
  bool SymbolTable::canImplicitlyConvert(Type const* from, Type const* to) {
    if (!conversions_.contains(from)) {
      return false;
    }

    const auto& options = conversions_[from];

    return std::ranges::find_if(options, [&](const Conversion& candidate) {
             return candidate.isImplicit && candidate.to == to;
           }) != options.end();
  }
  bool SymbolTable::canExplicitlyConvert(Type const* from, Type const* to) {
    if (!conversions_.contains(from)) {
      return false;
    }

    const auto& options = conversions_[from];
    return std::ranges::find_if(options, [&](const Conversion& candidate) { return candidate.to == to; }) !=
           options.end();
  }

  void SymbolTable::pushScope() { localScopes_.emplace(); }
  void SymbolTable::popScope() {
    if (!localScopes_.empty()) {
      localScopes_.pop();
    }
  }

  bool SymbolTable::addLocalVariable(ID id, Type const* type) {
    if (id == INVALID_ID) {
      return false;
    }
    auto& localScope = localScopes_.top();
    auto& variables = localScope.variables;
    variables.insert({id, type});
    return true;
  }

  Type const* SymbolTable::getLocalVariableType(ID id) const {
    auto& localScope = localScopes_.top();
    auto& variables = localScope.variables;
    if (id == INVALID_ID || !variables.contains(id)) {
      return nullptr;
    }

    return variables.at(id);
  }

  FunctionType const* SymbolTable::registerFunctionType(const FunctionType& func) {
    if (const auto i = functionTypes_.find(&func); i != functionTypes_.end()) {
      return i->get();
    }
    auto [i, _] = functionTypes_.insert(std::make_unique<FunctionType>(func));
    return i->get();
  }

  FunctionType const* SymbolTable::addFunction(std::string name, FunctionType func) {
    const auto funcType = registerFunctionType(func);
    functionNames_.emplace(std::move(name), funcType);
    return funcType;
  }

  FunctionType const* SymbolTable::getFunctionType(const std::string& name) const {
    auto it = functionNames_.find(name);
    if (it == functionNames_.end()) {
      return nullptr;
    }
    return it->second;
  }

  bool SymbolTable::isMagicBuiltin(const FunctionType* func) {
    if (!func) {
      return false;
    }
    // Criteria are no return and all parameters are MAGIC_ANY
    if (func->returnType() == &EMPTY() &&
        std::ranges::all_of(func->parameters(), [](types::Type const* p) { return p == &MAGIC_ANY_TYPE(); })) {
      return true;
    }

    return false;
  }

  size_t SymbolTable::SignatureHash::operator()(const FunctionType& func) const {
    size_t hash = 0;
    hash ^= std::hash<types::Type const*>{}(func.returnType());
    for (auto const& param : func.parameters()) {
      hash ^= std::hash<types::Type const*>{}(param);
    }
    return hash;
  }

  bool SymbolTable::SignatureEq::operator()(FunctionType const* lhs, FunctionType const* rhs) const {
    return lhs->returnType() == rhs->returnType() && std::ranges::equal(lhs->parameters(), rhs->parameters());
  }
}  // namespace fluir::types
