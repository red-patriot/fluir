#include "compiler/types/symbol_table.hpp"

#include <algorithm>

namespace fluir::types {
  SymbolTable::SymbolTable() : types_({{ID_INVALID, Type{""}}}), typeNames_({{"", ID_INVALID}}) { }

  TypeID SymbolTable::addType(Type t) {
    auto [id, added] = typeNames_.try_emplace(t.name(), static_cast<TypeID>(typeNames_.size()));
    if (added) {
      types_.insert({id->second, std::move(t)});
    }
    return id->second;
  }

  TypeID SymbolTable::getTypeID(const std::string& name) const {
    if (!name.empty() && typeNames_.contains(name)) {
      return typeNames_.at(name);
    }
    return TypeID::ID_INVALID;
  }

  Type const* SymbolTable::getType(TypeID id) const {
    if (id != ID_INVALID && types_.contains(id)) {
      return &types_.at(id);
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

  OperatorDefinition const* SymbolTable::selectOverload(TypeID lhs, Operator op, TypeID rhs) {
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

  OperatorDefinition const* SymbolTable::selectOverload(Operator op, TypeID operand) {
    /* 1. Get all candidates
     * 2. Filter out all binary operators
     * 3. Filter out all where there is no cast between given operand and operand
     */

    auto candidates = getOperatorOverloads(op);
    auto removed = std::ranges::remove_if(candidates, [](OperatorDefinition const* candidate) {
      return candidate->getParameters()[1] != TypeID::ID_INVALID;
    });
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

  void SymbolTable::addImplicitConversion(TypeID from, TypeID to) {
    if (from == to) {
      return;
    }
    Conversion c{.from = from, .to = to, .isImplicit = true};
    conversions_[from].insert(c);
  }
  void SymbolTable::addExplicitConversion(TypeID from, TypeID to) {
    if (from == to) {
      return;
    }

    conversions_[from].insert(Conversion{.from = from, .to = to, .isImplicit = false});
  }
  bool SymbolTable::canImplicitlyConvert(TypeID from, TypeID to) {
    if (!conversions_.contains(from)) {
      return false;
    }

    const auto& options = conversions_[from];

    return std::ranges::find_if(options, [&](const Conversion& candidate) {
             return candidate.isImplicit && candidate.to == to;
           }) != options.end();
  }
  bool SymbolTable::canExplicitlyConvert(TypeID from, TypeID to) {
    if (!conversions_.contains(from)) {
      return false;
    }

    const auto& options = conversions_[from];
    return std::ranges::find_if(options, [&](const Conversion& candidate) { return candidate.to == to; }) !=
           options.end();
  }
}  // namespace fluir::types
