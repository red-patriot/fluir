#include "compiler/types/symbol_table.hpp"

#include <algorithm>

namespace fluir::types {
  void SymbolTable::addType(Type t) { types_.emplace(t.name(), std::move(t)); }

  Type* SymbolTable::getType(const std::string& name) {
    if (types_.contains(name)) {
      return &types_.at(name);
    }
    return nullptr;
  }
  Type const* SymbolTable::getType(const std::string& name) const {
    if (types_.contains(name)) {
      return &types_.at(name);
    }
    return nullptr;
  }

  void SymbolTable::addOperator(Operator op) { operators_[op.getOperator()].insert(std::move(op)); }
  std::vector<Operator const*> SymbolTable::getOperatorOverloads(::fluir::Operator op) const {
    std::vector<Operator const*> result;
    std::ranges::transform(operators_.at(op), std::back_inserter(result), [](auto const& elem) { return &elem; });
    return result;
  }

}  // namespace fluir::types
