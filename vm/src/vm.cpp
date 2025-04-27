#include "vm/vm.hpp"

namespace fluir {
  ExecResult VirtualMachine::execute(code::ByteCode const* code) {
    // Reset the internal state
    code_ = code;
    ip_ = code_->chunks.at(0).code.data();  // TODO: Be smarter about loading the entry point

    return run();
  }

  ExecResult VirtualMachine::run() {
    using enum code::OpCode;
    for (;;) {
      code::OpCode instruction = *ip_++;
      switch (instruction) {
        case EXIT:
          goto afterLoop;
        default:
          return ExecResult::ERROR;
      }
    }
  afterLoop:
    return ExecResult::SUCCESS;
  }
}  // namespace fluir
