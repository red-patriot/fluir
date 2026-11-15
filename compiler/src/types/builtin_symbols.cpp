#include "compiler/types/builtin_symbols.hpp"

#include <array>
#include <string>

using namespace std::string_literals;

namespace fluir::types {
  SymbolTable buildSymbolTable() {
    SymbolTable table;
    instantiateBuiltinFunctions(table);
    instantiateBuiltinOperators(table);
    instantiateBuiltinCasts(table);
    return table;
  }

  void instantiateBuiltinOperators(SymbolTable& table) {
    const std::array signedTypes{&F64(), &I8(), &I16(), &I32(), &I64()};
    const std::array unsignedTypes{&U8(), &U16(), &U32(), &U64()};
    constexpr std::array binaryOps{Operator::PLUS, Operator::MINUS, Operator::STAR, Operator::SLASH};
    constexpr std::array unsignedUnaryOps{Operator::PLUS, Operator::PLUS_PLUS, Operator::MINUS_MINUS};
    constexpr std::array signedUnaryOps{Operator::MINUS, Operator::PLUS, Operator::PLUS_PLUS, Operator::MINUS_MINUS};
    constexpr std::array equalityOps{Operator::EQUAL_EQUAL, Operator::BANG_EQUAL};
    constexpr std::array orderingOps{Operator::LESS, Operator::GREATER, Operator::LESS_EQUAL, Operator::GREATER_EQUAL};

    for (const auto& type : signedTypes) {
      for (const auto& op : binaryOps) {
        table.addOperator(OperatorDefinition{type, op, type, type});
      }
      for (const auto& op : signedUnaryOps) {
        table.addOperator(OperatorDefinition{op, type, type});
      }
      for (const auto& op : equalityOps) {
        table.addOperator(OperatorDefinition{type, op, type, &BOOL()});
      }
      for (const auto& op : orderingOps) {
        table.addOperator(OperatorDefinition{type, op, type, &BOOL()});
      }
    }

    for (const auto& type : unsignedTypes) {
      for (const auto& op : binaryOps) {
        table.addOperator(OperatorDefinition{type, op, type, type});
      }
      for (const auto& op : unsignedUnaryOps) {
        table.addOperator(OperatorDefinition{op, type, type});
      }
      for (const auto& op : equalityOps) {
        table.addOperator(OperatorDefinition{type, op, type, &BOOL()});
      }
      for (const auto& op : orderingOps) {
        table.addOperator(OperatorDefinition{type, op, type, &BOOL()});
      }
    }

    {
      const auto type = &BOOL();
      table.addOperator(OperatorDefinition{Operator::BANG, type, type});
      table.addOperator(OperatorDefinition{type, Operator::AND_AND, type, type});
      table.addOperator(OperatorDefinition{type, Operator::BAR_BAR, type, type});
      for (const auto& op : equalityOps) {
        table.addOperator(OperatorDefinition{type, op, type, &BOOL()});
      }
    }
  }

  void instantiateBuiltinCasts(SymbolTable& table) {
    // TODO: Add builtin explicit casts
    static const std::array intCoercionChain{&I8(), &I16(), &I32(), &I64(), &F64()};

    for (auto source = intCoercionChain.begin();
         // use prev to not create coercions for F64
         source != std::prev(intCoercionChain.end());
         ++source) {
      for (auto target = std::next(source); target != intCoercionChain.end(); ++target) {
        table.addImplicitConversion(*source, *target);
      }
    }

    static const std::array uintCoercionChain{&U8(), &U16(), &U32(), &U64(), &F64()};
    for (auto source = uintCoercionChain.begin();
         // use prev to not create coercions for F64
         source != std::prev(uintCoercionChain.end());
         ++source) {
      for (auto target = std::next(source); target != uintCoercionChain.end(); ++target) {
        table.addImplicitConversion(*source, *target);
      }
    }
  }

  void instantiateBuiltinFunctions(SymbolTable& table) {
    table.addFunction("print"s, FunctionType{{&MAGIC_ANY_TYPE()}, &EMPTY()});
  }
}  // namespace fluir::types
