#include "compiler/types/builtin_symbols.hpp"

#include <array>
#include <string>

namespace fluir::types {
  SymbolTable buildSymbolTable() {
    SymbolTable table;
    instantiateBuiltinTypes(table);
    instantiateBuiltinOperators(table);
    instantiateBuiltinCasts(table);
    return table;
  }

  void instantiateBuiltinTypes(SymbolTable& table) {
    table.addType(Type("F64"));
    table.addType(Type("I8"));
    table.addType(Type("I16"));
    table.addType(Type("I32"));
    table.addType(Type("I64"));
    table.addType(Type("U8"));
    table.addType(Type("U16"));
    table.addType(Type("U32"));
    table.addType(Type("U64"));
  }

  void instantiateBuiltinOperators(SymbolTable& table) {
    std::array<std::string, 5> signedTypes = {"F64", "I8", "I16", "I32", "I64"};
    std::array<std::string, 4> unsignedTypes = {"U8", "U16", "U32", "U64"};
    std::array binaryOps{Operator::PLUS, Operator::MINUS, Operator::STAR, Operator::SLASH};
    std::array ussignedUnaryOps{Operator::PLUS};
    std::array signedUnaryOps{Operator::MINUS, Operator::PLUS};

    for (const auto& typeName : signedTypes) {
      auto type = table.getType(typeName);
      for (const auto& op : binaryOps) {
        table.addOperator(OperatorDefinition{type, op, type, type});
      }
      for (const auto& op : signedUnaryOps) {
        table.addOperator(OperatorDefinition{op, type, type});
      }
    }

    for (const auto& typeName : unsignedTypes) {
      auto type = table.getType(typeName);
      for (const auto& op : binaryOps) {
        table.addOperator(OperatorDefinition{type, op, type, type});
      }
      for (const auto& op : ussignedUnaryOps) {
        table.addOperator(OperatorDefinition{op, type, type});
      }
    }
  }

  void instantiateBuiltinCasts(SymbolTable& table) {
    // TODO: Add builtin explicit casts
    static const std::array<std::string, 5> intCoercionChain{"I8", "I16", "I32", "I64", "F64"};

    for (auto i = intCoercionChain.begin();
         // use prev to not create coercions for F64
         i != std::prev(intCoercionChain.end());
         ++i) {
      auto source = table.getType(*i);
      for (auto j = std::next(i); j != intCoercionChain.end(); ++j) {
        auto target = table.getType(*j);
        table.addImplicitConversion(source, target);
      }
    }

    static const std::array<std::string, 5> uintCoercionChain{"U8", "U16", "U32", "U64", "F64"};
    for (auto i = uintCoercionChain.begin();
         // use prev to not create coercions for F64
         i != std::prev(uintCoercionChain.end());
         ++i) {
      auto source = table.getType(*i);
      for (auto j = std::next(i); j != uintCoercionChain.end(); ++j) {
        auto target = table.getType(*j);
        table.addImplicitConversion(source, target);
      }
    }
  }
}  // namespace fluir::types
