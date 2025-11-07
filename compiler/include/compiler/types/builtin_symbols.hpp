#ifndef FLUIR_COMPILER_TYPES_BUILTIN_SYMBOLS_HPP
#define FLUIR_COMPILER_TYPES_BUILTIN_SYMBOLS_HPP

#include "compiler/types/symbol_table.hpp"

namespace fluir::types {
  void instantiateBuiltinTypes(SymbolTable& table);
  void instantiateBuiltinOperators(SymbolTable& table);
}  // namespace fluir::types

#endif
