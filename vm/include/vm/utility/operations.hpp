#ifndef FLUIR_VM_UTILITY_OPERATIONS_HPP
#define FLUIR_VM_UTILITY_OPERATIONS_HPP

namespace fluir::utility {
  template <typename T>
  struct increment {
    constexpr T operator()(T& x) const { return ++x; }
  };

  template <typename T>
  struct decrement {
    constexpr T operator()(T& x) const { return --x; }
  };

  inline code::NumericWidth widthof(code::PrimitiveType type) {
    switch (type) {
      case code::PrimitiveType::U8:
      case code::PrimitiveType::I8:
        return code::WIDTH_8;
      case code::PrimitiveType::U16:
      case code::PrimitiveType::I16:
        return code::WIDTH_16;
      case code::PrimitiveType::U32:
      case code::PrimitiveType::I32:
        return code::WIDTH_32;
      case code::PrimitiveType::U64:
      case code::PrimitiveType::I64:
      case code::PrimitiveType::F64:
        return code::WIDTH_64;
      default:
        throw VirtualMachineError("Invalid operation: Attempted to determine the width of a non-numeric type.");
    }
  }
}  // namespace fluir::utility

#endif
