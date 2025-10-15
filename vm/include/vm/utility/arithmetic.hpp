#ifndef FLUIR_VM_UTILITY_ARITHMETIC_HPP
#define FLUIR_VM_UTILITY_ARITHMETIC_HPP

#include <bytecode/value.hpp>

#include "vm/exceptions.hpp"

namespace fluir::utility {
  template <typename T>
  struct checkedDivide {
    T operator()(const T& lhs, const T& rhs) const {
      if (rhs == 0) {
        throw DivideByZeroError{"DIVISION BY ZERO"};
      }
      return lhs / rhs;
    }
  };

  inline code::I64 widenI(const code::Value& value, code::PrimitiveType& type) {
    type = value.type();
    switch (value.type()) {
      case code::PrimitiveType::I8:
        return static_cast<code::I64>(value.asI8());
      case code::PrimitiveType::I16:
        return static_cast<code::I64>(value.asI16());
      case code::PrimitiveType::I32:
        return static_cast<code::I64>(value.asI32());
      case code::PrimitiveType::I64:
        return value.asI64();
      default:
        break;
    }
    throw VirtualMachineError{"EXPECTED AN INT TYPE"};
  }

  inline code::Value narrowI(code::I64 val, const code::PrimitiveType& type) {
    switch (type) {
      case code::PrimitiveType::I8:
        return code::Value{static_cast<std::int8_t>(val)};
      case code::PrimitiveType::I16:
        return code::Value{static_cast<std::int16_t>(val)};
      case code::PrimitiveType::I32:
        return code::Value{static_cast<std::int32_t>(val)};
      case code::PrimitiveType::I64:
        return code::Value{val};
      default:
        break;
    }
    throw VirtualMachineError{"EXPECTED AN INT TYPE"};
  }

  inline code::U64 widenU(const code::Value& value, code::PrimitiveType& type) {
    type = value.type();
    switch (value.type()) {
      case code::PrimitiveType::U8:
        return static_cast<code::U64>(value.asU8());
      case code::PrimitiveType::U16:
        return static_cast<code::U64>(value.asU16());
      case code::PrimitiveType::U32:
        return static_cast<code::U64>(value.asU32());
      case code::PrimitiveType::U64:
        return value.asU64();
      default:
        break;
    }
    throw VirtualMachineError{"EXPECTED A UINT TYPE"};
  }

  inline code::Value narrowU(code::U64 val, const code::PrimitiveType& type) {
    switch (type) {
      case code::PrimitiveType::U8:
        return code::Value{static_cast<std::uint8_t>(val)};
      case code::PrimitiveType::U16:
        return code::Value{static_cast<std::uint16_t>(val)};
      case code::PrimitiveType::U32:
        return code::Value{static_cast<std::uint32_t>(val)};
      case code::PrimitiveType::U64:
        return code::Value{val};
      default:
        break;
    }
    throw VirtualMachineError{"EXPECTED A UINT TYPE"};
  }
}  // namespace fluir::utility

#endif
