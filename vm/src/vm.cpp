#include "vm/vm.hpp"

#include <iostream>

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
  }  // namespace

  ExecResult VirtualMachine::execute(code::ByteCode const* code) {
    // Reset the internal state
    stack_.clear();
    stack_.reserve(256);

    code_ = code;
    current_ = &code_->chunks.at(0);
    ip_ = current_->code.data();  // TODO: Be smarter about loading the entry point

    return run();
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
          {
            double rhs = stack_.back().asF64();
            stack_.pop_back();
            double lhs = stack_.back().asF64();
            stack_.pop_back();
            stack_.emplace_back(code::Value{lhs + rhs});
            break;
          }
        case F64_SUB:
          {
            double rhs = stack_.back().asF64();
            stack_.pop_back();
            double lhs = stack_.back().asF64();
            stack_.pop_back();
            stack_.emplace_back(lhs - rhs);
            break;
          }
        case F64_MUL:
          {
            double rhs = stack_.back().asF64();
            stack_.pop_back();
            double lhs = stack_.back().asF64();
            stack_.pop_back();
            stack_.emplace_back(lhs * rhs);
            break;
          }
        case F64_DIV:
          {
            double rhs = stack_.back().asF64();
            stack_.pop_back();
            double lhs = stack_.back().asF64();
            stack_.pop_back();
            stack_.emplace_back(lhs / rhs);
            break;
          }
        case F64_AFF:
          {
            double operand = stack_.back().asF64();
            stack_.pop_back();
            stack_.emplace_back(+operand);
            break;
          }
        case F64_NEG:
          {
            double operand = stack_.back().asF64();
            stack_.pop_back();
            stack_.emplace_back(-operand);
            break;
          }
        case I64_ADD:
          {
            std::int64_t rhs = stack_.back().asI64();
            stack_.pop_back();
            std::int64_t lhs = stack_.back().asI64();
            stack_.pop_back();
            stack_.emplace_back(lhs + rhs);
            break;
          }
        case I64_SUB:
          {
            std::int64_t rhs = stack_.back().asI64();
            stack_.pop_back();
            std::int64_t lhs = stack_.back().asI64();
            stack_.pop_back();
            stack_.emplace_back(lhs - rhs);
            break;
          }
        case I64_MUL:
          {
            std::int64_t rhs = stack_.back().asI64();
            stack_.pop_back();
            std::int64_t lhs = stack_.back().asI64();
            stack_.pop_back();
            stack_.emplace_back(lhs * rhs);
            break;
          }
        case I64_DIV:
          {
            std::int64_t rhs = stack_.back().asI64();
            stack_.pop_back();
            std::int64_t lhs = stack_.back().asI64();
            stack_.pop_back();
            stack_.emplace_back(lhs / rhs);
            break;
          }
        case I64_NEG:
          {
            std::int64_t operand = stack_.back().asI64();
            stack_.pop_back();
            stack_.emplace_back(-operand);
            break;
          }
        case I64_AFF:
          {
            std::int64_t operand = stack_.back().asI64();
            stack_.pop_back();
            stack_.emplace_back(+operand);
            break;
          }
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

#undef FLUIR_READ_BYTE
  }
}  // namespace fluir
