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
    constexpr std::array signedTypes{types::ID_F64, types::ID_I8, types::ID_I16, types::ID_I32, types::ID_I64};
    constexpr std::array unsignedTypes{types::ID_U8, types::ID_U16, types::ID_U32, types::ID_U64};
    constexpr std::array binaryOps{Operator::PLUS, Operator::MINUS, Operator::STAR, Operator::SLASH};
    constexpr std::array unsignedUnaryOps{Operator::PLUS};
    constexpr std::array signedUnaryOps{Operator::MINUS, Operator::PLUS};

    for (const auto& type : signedTypes) {
      for (const auto& op : binaryOps) {
        table.addOperator(OperatorDefinition{type, op, type, type});
      }
      for (const auto& op : signedUnaryOps) {
        table.addOperator(OperatorDefinition{op, type, type});
      }
    }

    for (const auto& type : unsignedTypes) {
      for (const auto& op : binaryOps) {
        table.addOperator(OperatorDefinition{type, op, type, type});
      }
      for (const auto& op : unsignedUnaryOps) {
        table.addOperator(OperatorDefinition{op, type, type});
      }
    }
  }

  void instantiateBuiltinCasts(SymbolTable& table) {
    // TODO: Add builtin explicit casts
    static constexpr std::array intCoercionChain{
      types::ID_I8, types::ID_I16, types::ID_I32, types::ID_I64, types::ID_F64};

    for (auto source = intCoercionChain.begin();
         // use prev to not create coercions for F64
         source != std::prev(intCoercionChain.end());
         ++source) {
      for (auto target = std::next(source); target != intCoercionChain.end(); ++target) {
        table.addImplicitConversion(*source, *target);
      }
    }

    static constexpr std::array uintCoercionChain{
      types::ID_U8, types::ID_U16, types::ID_U32, types::ID_U64, types::ID_F64};
    for (auto source = uintCoercionChain.begin();
         // use prev to not create coercions for F64
         source != std::prev(uintCoercionChain.end());
         ++source) {
      for (auto target = std::next(source); target != uintCoercionChain.end(); ++target) {
        table.addImplicitConversion(*source, *target);
      }
    }
  }
}  // namespace fluir::types
