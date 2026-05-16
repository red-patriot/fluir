#ifndef FLUIR_COMPILER_TYPES_TYPEID_HPP
#define FLUIR_COMPILER_TYPES_TYPEID_HPP

#include <cstdint>

namespace fluir::types {
  /** A unique identifier for a type in the compiler */
  enum TypeID : std::uint64_t {
    ID_INVALID = 0,
    ID_F64,
    ID_I8,
    ID_I16,
    ID_I32,
    ID_I64,
    ID_U8,
    ID_U16,
    ID_U32,
    ID_U64,
    ID_MAGIC_ANY_TYPE,  // HACK: this is only used as an input type for builtins that handle typing "magically"
                        // It represents a data sink and can accept any type
                        // IT IS IMPERATIVE THIS NEVER BE ALLOWED TO LEAK INTO USER CODE
                        // TODO: remove this once things are updated to support generics
  };

}  // namespace fluir::types

#endif
