#include "../../include/vm/machine/vm.hpp"

#include <algorithm>
#include <format>  // Use format in VM instead of fmt to reduce dependencies of the runtime
#include <functional>
#include <iostream>
#include <stack>

#include "vm/exceptions.hpp"
#include "vm/utility/narrow_widen.hpp"
#include "vm/utility/operations.hpp"

namespace fluir {
  namespace {
    // TODO: Remove this later
    // This code is just for debugging purposes until the rest of the
    // language is implemented
    std::ostream& operator<<(std::ostream& os, const code::Value& value) {
      switch (value.type()) {
        case code::PrimitiveType::EMPTY:
          os << "<NULL>";
          break;
        case code::PrimitiveType::F64:
          os << "(F64)" << value.asF64();
          break;
        case code::PrimitiveType::I8:
          os << "(I8)" << value.asI8();
          break;
        case code::PrimitiveType::I16:
          os << "(I16)" << value.asI16();
          break;
        case code::PrimitiveType::I32:
          os << "(I32)" << value.asI32();
          break;
        case code::PrimitiveType::I64:
          os << "(I64)" << value.asI64();
          break;
        case code::PrimitiveType::U8:
          os << "(U8)" << value.asU8();
          break;
        case code::PrimitiveType::U16:
          os << "(U16)" << value.asU16();
          break;
        case code::PrimitiveType::U32:
          os << "(U32)" << value.asU32();
          break;
        case code::PrimitiveType::U64:
          os << "(U64)" << value.asU64();
          break;
      }

      return os;
    }
  }  // namespace

  template <typename Op>
  void VirtualMachine::floatBinary() {
    double rhs = stackTop().asF64();
    popStack();
    double lhs = stackTop().asF64();
    popStack();
    pushStack(code::Value{Op{}(lhs, rhs)});
  }
  template <typename Op>
  void VirtualMachine::floatUnary() {
    double operand = stackTop().asF64();
    popStack();
    pushStack(code::Value(Op{}(operand)));
  }
  template <typename Op>
  void VirtualMachine::intBinary() {
    code::PrimitiveType typeR, typeL;
    code::I64 rhs = utility::widenI(stackTop(), typeR);
    popStack();
    code::I64 lhs = utility::widenI(stackTop(), typeL);
    popStack();
    pushStack(utility::narrowI(Op{}(lhs, rhs), std::max(typeR, typeL)));
  }
  template <typename Op>
  void VirtualMachine::intUnary() {
    code::PrimitiveType type;
    code::I64 operand = utility::widenI(stackTop(), type);
    popStack();
    pushStack(utility::narrowI(Op{}(operand), type));
  }
  template <typename Op>
  void VirtualMachine::uintBinary() {
    code::PrimitiveType typeR, typeL;
    code::U64 rhs = utility::widenU(stackTop(), typeR);
    popStack();
    code::U64 lhs = utility::widenU(stackTop(), typeL);
    popStack();
    pushStack(utility::narrowU(Op{}(lhs, rhs), std::max(typeR, typeL)));
  }
  template <typename Op>
  void VirtualMachine::uintUnary() {
    code::PrimitiveType type;
    code::U64 operand = utility::widenU(stackTop(), type);
    popStack();
    pushStack(utility::narrowU(Op{}(operand), type));
  }

  ExecResult VirtualMachine::execute(code::ByteCode const* code) {
    // Reset the internal state
    if (!stack_) {
      stack_ = std::make_unique<Stack>();
    }
    std::ranges::fill(*stack_, code::Value{});
    stackBegin_ = stack_->data();
    stackEnd_ = stack_->data();

    code_ = code;
    current_ = &code_->chunks.at(0);
    ip_ = current_->code.data();  // TODO: Be smarter about loading the entry point

    try {
      return run();
    } catch (const DivideByZeroError& e) {
      std::cerr << e.what() << std::endl;
      return ExecResult::ERROR_DIVIDE_BY_ZERO;
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

    using enum code::Instruction;
    for (;;) {
      std::uint8_t instruction = EXIT;
      switch (instruction = FLUIR_READ_BYTE()) {
        case PUSH:
          {
            uint8_t index = FLUIR_READ_BYTE();
            const code::Value& val = current_->constants[index];
            if (stackSize() >= STACK_LIMIT) {
              return ExecResult::ERROR;
            }
            pushStack(val);
            break;
          }
        case GET_VAL:
          {
            const auto index = FLUIR_READ_BYTE();
            if (stackSize() >= STACK_LIMIT) {
              return ExecResult::ERROR;
            }
            pushStack(stackBegin_[index]);
            break;
          }
        case SET_VAL:
          {
            const auto index = FLUIR_READ_BYTE();
            stackBegin_[index] = stackTop();
            break;
          }
        case F64_ADD:
          floatBinary<std::plus<code::F64>>();
          break;
        case F64_SUB:
          floatBinary<std::minus<code::F64>>();
          break;
        case F64_MUL:
          floatBinary<std::multiplies<code::F64>>();
          break;
        case F64_DIV:
          floatBinary<std::divides<code::F64>>();
          break;
        case F64_NEG:
          floatUnary<std::negate<code::F64>>();
          break;
        case F64_INC:
          floatUnary<utility::increment<code::F64>>();
          break;
        case F64_DEC:
          floatUnary<utility::decrement<code::F64>>();
          break;
        case I64_ADD:
          intBinary<std::plus<code::I64>>();
          break;
        case I64_SUB:
          intBinary<std::minus<code::I64>>();
          break;
        case I64_MUL:
          intBinary<std::multiplies<code::I64>>();
          break;
        case I64_DIV:
          intBinary<utility::checkedDivide<code::I64>>();
          break;
        case I64_INC:
          intUnary<utility::increment<code::I64>>();
          break;
        case I64_DEC:
          intUnary<utility::decrement<code::I64>>();
          break;
        case I64_NEG:
          intUnary<std::negate<code::I64>>();
          break;
        case U64_ADD:
          uintBinary<std::plus<code::U64>>();
          break;
        case U64_SUB:
          uintBinary<std::minus<code::U64>>();
          break;
        case U64_MUL:
          uintBinary<std::multiplies<code::U64>>();
          break;
        case U64_DIV:
          uintBinary<utility::checkedDivide<code::U64>>();
          break;
        case U64_INC:
          uintUnary<utility::increment<code::U64>>();
          break;
        case U64_DEC:
          uintUnary<utility::decrement<code::U64>>();
          break;
        case F64_AFF:
        case I64_AFF:
        case U64_AFF:
          break;  // This is a No-Op
        case CAST_IU:
          {
            auto width = static_cast<code::NumericWidth>(FLUIR_READ_BYTE());
            auto toCast = stackTop();
            popStack();
            code::PrimitiveType _;
            auto widened = utility::widenI(toCast, _);
            auto casted = static_cast<code::U64>(widened);

            pushStack(utility::narrowU(casted, static_cast<code::PrimitiveType>(code::UNSIGNED | width)));
          }
          break;
        case CAST_UI:
          {
            auto width = static_cast<code::NumericWidth>(FLUIR_READ_BYTE());
            auto toCast = stackTop();
            popStack();
            code::PrimitiveType _;
            auto widened = utility::widenU(toCast, _);
            auto casted = static_cast<code::I64>(widened);
            pushStack(utility::narrowI(casted, static_cast<code::PrimitiveType>(code::SIGNED | width)));
          }
          break;
        case CAST_IF:
          {
            auto toCast = stackTop();
            popStack();
            code::PrimitiveType _;
            auto widened = utility::widenI(toCast, _);
            auto casted = static_cast<code::F64>(widened);
            pushStack(code::Value{casted});
          }
          break;
        case CAST_FI:
          {
            auto width = static_cast<code::NumericWidth>(FLUIR_READ_BYTE());
            auto toCast = stackTop();
            popStack();
            auto casted = static_cast<code::I64>(toCast.asF64());
            pushStack(utility::narrowI(casted, static_cast<code::PrimitiveType>(code::SIGNED | width)));
          }
          break;
        case CAST_UF:
          {
            auto toCast = stackTop();
            popStack();
            code::PrimitiveType _;
            auto widened = utility::widenU(toCast, _);
            auto casted = static_cast<code::F64>(widened);
            pushStack(code::Value{casted});
          }
          break;
        case CAST_FU:
          {
            auto width = static_cast<code::NumericWidth>(FLUIR_READ_BYTE());
            auto toCast = stackTop();
            popStack();
            auto casted = static_cast<code::U64>(toCast.asF64());
            pushStack(utility::narrowU(casted, static_cast<code::PrimitiveType>(code::UNSIGNED | width)));
          }
          break;
        case POP:
          // TODO: Remove this later
          // This code is just for debugging purposes until the rest of the
          // language is implemented
          std::cout << stackTop() << '\n';
          popStack();
          break;
        case MULTIPOP:
          {
            auto count = FLUIR_READ_BYTE();
            for (size_t i = 0; i != count; ++i) {
              popStack();
            }
          }
        case EXIT:
          // TODO: Clean up this testing code later...
          goto afterLoop;
        default:
          return ExecResult::ERROR;
      }
    }
  afterLoop:
    return ExecResult::SUCCESS;
  }

  code::Value& VirtualMachine::stackTop() { return *(stackEnd_ - 1); }
  void VirtualMachine::popStack() { --stackEnd_; }
  void VirtualMachine::pushStack(code::Value value) {
    (*stackEnd_) = std::move(value);
    ++stackEnd_;
  }
}  // namespace fluir
