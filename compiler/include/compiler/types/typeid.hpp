#ifndef FLUIR_COMPILER_TYPES_TYPEID_HPP
#define FLUIR_COMPILER_TYPES_TYPEID_HPP

#include <cstdint>

namespace fluir::types {
  /** A unique identifier for a type in the compiler */
  enum TypeID : std::uint64_t { ID_INVALID = 0, ID_F64, ID_I8, ID_I16, ID_I32, ID_I64, ID_U8, ID_U16, ID_U32, ID_U64 };

}  // namespace fluir::types

#endif
