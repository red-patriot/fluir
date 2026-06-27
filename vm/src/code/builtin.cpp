#include "vm/code/builtin.hpp"

#include <iostream>

#include "vm/code/value.hpp"

namespace fluir {
  using namespace std::string_view_literals;

  NativeFunctionsMap getBuiltins() {
    NativeFunctionsMap builtins{{"print", print}};
    return builtins;
  }

  void print(CallFrame& frame) {
    std::cout << stackTop(frame) << '\n';

    // Clean up this functions frame
    pop(frame);
  }

  std::ostream& operator<<(std::ostream& os, const code::Value& value) {
    switch (value.type()) {
      case code::PrimitiveType::EMPTY:
        os << "NULL";
        break;
      case code::PrimitiveType::F64:
        os << value.asF64();
        break;
      case code::PrimitiveType::I8:
        os << value.asI8();
        break;
      case code::PrimitiveType::I16:
        os << value.asI16();
        break;
      case code::PrimitiveType::I32:
        os << value.asI32();
        break;
      case code::PrimitiveType::I64:
        os << value.asI64();
        break;
      case code::PrimitiveType::U8:
        os << value.asU8();
        break;
      case code::PrimitiveType::U16:
        os << value.asU16();
        break;
      case code::PrimitiveType::U32:
        os << value.asU32();
        break;
      case code::PrimitiveType::U64:
        os << value.asU64();
        break;
      case code::PrimitiveType::STR:
        os << value.asStr().view();
        break;
      case code::PrimitiveType::BOOL:
        os << (value.asBool() ? "true"sv : "false"sv);
        break;
    }

    return os;
  }

}  // namespace fluir
