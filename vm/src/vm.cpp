#include "vm/vm.hpp"

#include <algorithm>
#include <format>  // Use format in VM instead of fmt to reduce dependencies of the runtime
#include <iostream>

#include "vm/exceptions.hpp"

namespace fluir {
  namespace {
    // TODO: Remove this later
    // This code is just for debugging purposes until the rest of the
    // language is implemented
    std::ostream& operator<<(std::ostream& os, const code::Value& value) {
      switch (value.type()) {
#define FLUIR_PRINT_VALUE(Type, Concrete)          \
  case code::ValueType::Type:                      \
    os << '(' << #Type << ')' << value.as##Type(); \
    break;

        FLUIR_CODE_VALUE_TYPES(FLUIR_PRINT_VALUE)
#undef FLUIR_PRINT_VALUE
      }

      return os;
    }

    int64_t widenI(const code::Value& value, code::ValueType& type) {
      type = value.type();
      switch (value.type()) {
        case code::ValueType::I8:
          return static_cast<std::int64_t>(value.asI8());
        case code::ValueType::I16:
          return static_cast<std::int64_t>(value.asI16());
        case code::ValueType::I32:
          return static_cast<std::int64_t>(value.asI32());
        case code::ValueType::I64:
          return value.asI64();
        default:
          break;
      }
      throw VirtualMachineError{"EXPECTED AN INT TYPE"};
    }

    code::Value narrowI(std::int64_t val, const code::ValueType& type) {
      switch (type) {
        case code::ValueType::I8:
          return code::Value{static_cast<std::int8_t>(val)};
        case code::ValueType::I16:
          return code::Value{static_cast<std::int16_t>(val)};
        case code::ValueType::I32:
          return code::Value{static_cast<std::int32_t>(val)};
        case code::ValueType::I64:
          return code::Value{val};
        default:
          break;
      }
      throw VirtualMachineError{"EXPECTED AN INT TYPE"};
    }

    uint64_t widenU(const code::Value& value, code::ValueType& type) {
      type = value.type();
      switch (value.type()) {
        case code::ValueType::U8:
          return static_cast<std::uint64_t>(value.asU8());
        case code::ValueType::U16:
          return static_cast<std::uint64_t>(value.asU16());
        case code::ValueType::U32:
          return static_cast<std::uint64_t>(value.asU32());
        case code::ValueType::U64:
          return value.asU64();
        default:
          break;
      }
      throw VirtualMachineError{"EXPECTED A UINT TYPE"};
    }

    code::Value narrowU(std::uint64_t val, const code::ValueType& type) {
      switch (type) {
        case code::ValueType::U8:
          return code::Value{static_cast<std::uint8_t>(val)};
        case code::ValueType::U16:
          return code::Value{static_cast<std::uint16_t>(val)};
        case code::ValueType::U32:
          return code::Value{static_cast<std::uint32_t>(val)};
        case code::ValueType::U64:
          return code::Value{val};
        default:
          break;
      }
      throw VirtualMachineError{"EXPECTED A UINT TYPE"};
    }
  }  // namespace

  ExecResult VirtualMachine::execute(code::ByteCode const* code) {
    // Reset the internal state
    stack_.clear();
    stack_.reserve(256);

    code_ = code;
    current_ = &code_->chunks.at(0);
    ip_ = current_->code.data();  // TODO: Be smarter about loading the entry point

    try {
      return run();
    } catch (const VirtualMachineError& e) {
      std::cerr << e.what() << std::endl;
      return ExecResult::ERROR;
    } catch (const std::exception& e) {
      std::cerr << "An unexpected error occurred: \n" << e.what() << std::endl;
      return ExecResult::ERROR;
    } catch (...) {
      std::cerr << "VM PANIC" << std::endl;
      return ExecResult::ERROR;
    }
  }

  ExecResult VirtualMachine::run() {
#define FLUIR_READ_BYTE() *ip_++

#define FLUIR_FLOAT_BINARY(OP)                    \
  {                                               \
    double rhs = stack_.back().asF64();           \
    stack_.pop_back();                            \
    double lhs = stack_.back().asF64();           \
    stack_.pop_back();                            \
    stack_.emplace_back(code::Value{lhs OP rhs}); \
    break;                                        \
  }

#define FLUIR_FLOAT_UNARY(OP)               \
  {                                         \
    double operand = stack_.back().asF64(); \
    stack_.pop_back();                      \
    stack_.emplace_back(OP operand);        \
    break;                                  \
  }

#define FLUIR_INT_BINARY(OP)                                          \
  {                                                                   \
    code::ValueType typeR, typeL;                                     \
    std::int64_t rhs = widenI(stack_.back(), typeR);                  \
    stack_.pop_back();                                                \
    std::int64_t lhs = widenI(stack_.back(), typeL);                  \
    stack_.pop_back();                                                \
    stack_.emplace_back(narrowI(lhs OP rhs, std::max(typeR, typeL))); \
    break;                                                            \
  }

#define FLUIR_INT_UNARY(OP)                               \
  {                                                       \
    code::ValueType typeOp;                               \
    std::int64_t operand = widenI(stack_.back(), typeOp); \
    stack_.pop_back();                                    \
    stack_.emplace_back(narrowI(OP operand, typeOp));     \
    break;                                                \
  }

#define FLUIR_UINT_BINARY(OP)                                         \
  {                                                                   \
    code::ValueType typeR, typeL;                                     \
    std::uint64_t rhs = widenU(stack_.back(), typeR);                 \
    stack_.pop_back();                                                \
    std::uint64_t lhs = widenU(stack_.back(), typeL);                 \
    stack_.pop_back();                                                \
    stack_.emplace_back(narrowU(lhs OP rhs, std::max(typeR, typeL))); \
    break;                                                            \
  }

#define FLUIR_UINT_UNARY(OP)                               \
  {                                                        \
    code::ValueType typeOp;                                \
    std::uint64_t operand = widenU(stack_.back(), typeOp); \
    stack_.pop_back();                                     \
    stack_.emplace_back(narrowU(OP operand, typeOp));      \
    break;                                                 \
  }

    using enum code::Instruction;
    for (;;) {
      std::uint8_t instruction = EXIT;
      switch (instruction = FLUIR_READ_BYTE()) {
        case PUSH:
          {
            uint8_t index = FLUIR_READ_BYTE();
            const code::Value& val = current_->constants[index];
            if (!(stack_.size() < 256)) {
              return ExecResult::ERROR;
            }
            stack_.emplace_back(val);
            break;
          }
        case F64_ADD:
          FLUIR_FLOAT_BINARY(+)
        case F64_SUB:
          FLUIR_FLOAT_BINARY(-)
        case F64_MUL:
          FLUIR_FLOAT_BINARY(*)
        case F64_DIV:
          FLUIR_FLOAT_BINARY(/)
        case F64_NEG:
          FLUIR_FLOAT_UNARY(-)
        case F64_AFF:
          FLUIR_FLOAT_UNARY(+)
        case I64_ADD:
          FLUIR_INT_BINARY(+)
        case I64_SUB:
          FLUIR_INT_BINARY(-)
        case I64_MUL:
          FLUIR_INT_BINARY(*)
        case I64_DIV:
          FLUIR_INT_BINARY(/)
        case I64_NEG:
          FLUIR_INT_UNARY(-)
        case I64_AFF:
          FLUIR_INT_UNARY(+)
        case U64_ADD:
          FLUIR_UINT_BINARY(+)
        case U64_SUB:
          FLUIR_UINT_BINARY(-)
        case U64_MUL:
          FLUIR_UINT_BINARY(*)
        case U64_DIV:
          FLUIR_UINT_BINARY(/)
        case U64_NEG:
          FLUIR_UINT_UNARY(-)
        case U64_AFF:
          FLUIR_UINT_UNARY(+)

        case POP:
          // TODO: Remove this later
          // This code is just for debugging purposes until the rest of the
          // language is implemented
          std::cout << stack_.back() << '\n';
          stack_.pop_back();
          break;
        case EXIT:
          // TODO: Clean up this testing code later...
          goto afterLoop;
        default:
          return ExecResult::ERROR;
      }
    }
  afterLoop:
    return ExecResult::SUCCESS;

#undef FLUIR_UINT_UNARY
#undef FLUIR_UINT_BINARY
#undef FLUIR_INT_UNARY
#undef FLUIR_INT_BINARY
#undef FLUIR_FLOAT_UNARY
#undef FLUIR_FLOAT_BINARY
#undef FLUIR_READ_BYTE
  }  // namespace fluir
}  // namespace fluir
