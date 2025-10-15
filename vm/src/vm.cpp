#include "vm/vm.hpp"

#include <algorithm>
#include <format>  // Use format in VM instead of fmt to reduce dependencies of the runtime
#include <functional>
#include <iostream>

#include "vm/exceptions.hpp"
#include "vm/utility/arithmetic.hpp"

namespace fluir {
  namespace {
    // TODO: Remove this later
    // This code is just for debugging purposes until the rest of the
    // language is implemented
    std::ostream& operator<<(std::ostream& os, const code::Value& value) {
      switch (value.type()) {
#define FLUIR_PRINT_VALUE(Type, Concrete)          \
  case code::PrimitiveType::Type:                  \
    os << '(' << #Type << ')' << value.as##Type(); \
    break;

        FLUIR_CODE_PRIMITIVE_TYPES(FLUIR_PRINT_VALUE)
#undef FLUIR_PRINT_VALUE
      }

      return os;
    }
  }  // namespace

  template <typename Op>
  void VirtualMachine::floatBinary() {
    double rhs = stack_.back().asF64();
    stack_.pop_back();
    double lhs = stack_.back().asF64();
    stack_.pop_back();
    stack_.emplace_back(code::Value{Op{}(lhs, rhs)});
  }
  template <typename Op>
  void VirtualMachine::floatUnary() {
    double operand = stack_.back().asF64();
    stack_.pop_back();
    stack_.emplace_back(Op{}(operand));
  }
  template <typename Op>
  void VirtualMachine::intBinary() {
    code::PrimitiveType typeR, typeL;
    code::I64 rhs = utility::widenI(stack_.back(), typeR);
    stack_.pop_back();
    code::I64 lhs = utility::widenI(stack_.back(), typeL);
    stack_.pop_back();
    stack_.emplace_back(utility::narrowI(Op{}(lhs, rhs), std::max(typeR, typeL)));
  }
  template <typename Op>
  void VirtualMachine::intUnary() {
    code::PrimitiveType type;
    code::I64 operand = utility::widenI(stack_.back(), type);
    stack_.pop_back();
    stack_.emplace_back(utility::narrowI(Op{}(operand), type));
  }
  template <typename Op>
  void VirtualMachine::uintBinary() {
    code::PrimitiveType typeR, typeL;
    code::U64 rhs = utility::widenU(stack_.back(), typeR);
    stack_.pop_back();
    code::U64 lhs = utility::widenU(stack_.back(), typeL);
    stack_.pop_back();
    stack_.emplace_back(utility::narrowU(Op{}(lhs, rhs), std::max(typeR, typeL)));
  }
  template <typename Op>
  void VirtualMachine::uintUnary() {
    code::PrimitiveType type;
    code::U64 operand = utility::widenU(stack_.back(), type);
    stack_.pop_back();
    stack_.emplace_back(utility::narrowU(Op{}(operand), type));
  }

  ExecResult VirtualMachine::execute(code::ByteCode const* code) {
    // Reset the internal state
    stack_.clear();
    stack_.reserve(256);

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
            if (!(stack_.size() < 256)) {
              return ExecResult::ERROR;
            }
            stack_.emplace_back(val);
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
        case F64_AFF:
        case I64_AFF:
        case U64_AFF:
          break;  // This is a No-Op
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
  }
}  // namespace fluir
