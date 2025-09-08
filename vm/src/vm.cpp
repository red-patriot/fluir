#include "vm/vm.hpp"

#include <iostream>

namespace fluir {
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
        case PUSH_FP:
          {
            uint8_t index = FLUIR_READ_BYTE();
            const code::Value& val = current_->constants[index];
            if (!(stack_.size() < 256)) {
              return ExecResult::ERROR;
            }
            stack_.push_back(val);
            break;
          }
        case F64_ADD:
          {
            double rhs = stack_.back();
            stack_.pop_back();
            double lhs = stack_.back();
            stack_.pop_back();
            stack_.push_back(lhs + rhs);
            break;
          }
        case F64_SUB:
          {
            double rhs = stack_.back();
            stack_.pop_back();
            double lhs = stack_.back();
            stack_.pop_back();
            stack_.push_back(lhs - rhs);
            break;
          }
        case F64_MUL:
          {
            double rhs = stack_.back();
            stack_.pop_back();
            double lhs = stack_.back();
            stack_.pop_back();
            stack_.push_back(lhs * rhs);
            break;
          }
        case F64_DIV:
          {
            double rhs = stack_.back();
            stack_.pop_back();
            double lhs = stack_.back();
            stack_.pop_back();
            stack_.push_back(lhs / rhs);
            break;
          }
        case F64_AFF:
          {
            double operand = stack_.back();
            stack_.pop_back();
            stack_.push_back(+operand);
            break;
          }
        case F64_NEG:
          {
            double operand = stack_.back();
            stack_.pop_back();
            stack_.push_back(-operand);
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
