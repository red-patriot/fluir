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

  Operator const* SymbolTable::addOperator(Operator op) {
    auto [it, added] = operators_[op.getOperator()].insert(std::move(op));
    return &*it;
  }
  std::vector<Operator const*> SymbolTable::getOperatorOverloads(::fluir::Operator op) const {
    std::vector<Operator const*> result;
    std::ranges::transform(operators_.at(op), std::back_inserter(result), [](auto const& elem) { return &elem; });
    return result;
  }

  void SymbolTable::addCast(Type const* from, Type const* to) {
    if (from == to) {
      return;
    }
    Conversion c{.from = from, .to = to, .isImplicit = true};
    conversions_[from].insert(c);
  }
  void SymbolTable::addConversion(Type const* from, Type const* to) {
    if (from == to) {
      return;
    }

    conversions_[from].insert(Conversion{.from = from, .to = to, .isImplicit = false});
  }
  bool SymbolTable::canCast(Type const* from, Type const* to) {
    if (!conversions_.contains(from)) {
      return false;
    }

    const auto& options = conversions_[from];

    return std::ranges::find_if(options, [&](const Conversion& candidate) {
             return candidate.isImplicit && candidate.to == to;
           }) != options.end();
  }
  bool SymbolTable::canConvert(Type const* from, Type const* to) {
    if (!conversions_.contains(from)) {
      return false;
    }

    const auto& options = conversions_[from];
    return std::ranges::find_if(options, [&](const Conversion& candidate) { return candidate.to == to; }) !=
           options.end();
  }
}  // namespace fluir::types
