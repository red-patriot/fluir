#include "compiler/types/symbol_table.hpp"

#include <algorithm>

namespace fluir::types {
  Type const* SymbolTable::addType(Type t) {
    auto [it, added] = types_.try_emplace(t.name(), std::move(t));
    return &it->second;
  }

  Type const* SymbolTable::getType(const std::string& name) const {
    if (types_.contains(name)) {
      return &types_.at(name);
    }
    return nullptr;
  }

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
      candidates, [](OperatorDefinition const* candidate) { return candidate->getParameters()[1] != nullptr; });
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
}  // namespace fluir::types
