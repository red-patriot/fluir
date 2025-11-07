#include "compiler/types/builtin_symbols.hpp"

namespace fluir::types {
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

}  // namespace fluir::types
