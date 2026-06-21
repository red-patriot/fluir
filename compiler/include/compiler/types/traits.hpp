#ifndef FLUIR_COMPILER_TYPES_TRAITS_HPP
#define FLUIR_COMPILER_TYPES_TRAITS_HPP

#include "bytecode/primitives.hpp"
#include "compiler/types/typeid.hpp"

namespace fluir::types {
  /* Indicates if the type given is an integral type */
  inline bool isIntegral(TypeID type) {
    switch (type) {
      case ID_I8:
      case ID_I16:
      case ID_I32:
      case ID_I64:
      case ID_U8:
      case ID_U16:
      case ID_U32:
      case ID_U64:
        return true;
      default:
        return false;
    }
  }
  /* Indicates if the type given is a floating-point type*/
  inline bool isFloatingPoint(TypeID type) { return type == ID_F64; }

  /* Indicates if the type given is a signed type */
  inline bool isSigned(TypeID type) {
    switch (type) {
      case ID_I8:
      case ID_I16:
      case ID_I32:
      case ID_I64:
      case ID_F64:
        return true;
      default:
        return false;
    }
  }
  /* Indicates if the type given is an unsigned type */
  inline bool isUnsigned(TypeID type) {
    switch (type) {
      case ID_U8:
      case ID_U16:
      case ID_U32:
      case ID_U64:
        return true;
      default:
        return false;
    }
  }

  inline fluir::code::NumericWidth widthOf(TypeID type) {
    switch (type) {
      case ID_F64:
      case ID_U64:
      case ID_I64:
        return code::NumericWidth::WIDTH_64;
      case ID_U32:
      case ID_I32:
        return code::NumericWidth::WIDTH_32;
      case ID_U16:
      case ID_I16:
        return code::NumericWidth::WIDTH_16;
      case ID_U8:
      case ID_I8:
        return code::NumericWidth::WIDTH_8;
      default:
        return code::NumericWidth{0};
    }
  }
}  // namespace fluir::types

#endif
