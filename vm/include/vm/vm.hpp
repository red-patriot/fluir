#ifndef FLUIR_VM_CM_HPP
#define FLUIR_VM_CM_HPP

#include <bytecode/byte_code.hpp>

namespace fluir {
  enum class ExecResult {
    SUCCESS,
    ERROR
  };

  class VirtualMachine {
   public:
    VirtualMachine() = default;
    VirtualMachine(const VirtualMachine&) = delete;
    VirtualMachine& operator=(const VirtualMachine&) = delete;
    VirtualMachine(VirtualMachine&&) = delete;
    VirtualMachine& operator=(VirtualMachine&&) = delete;
    ~VirtualMachine() = default;

    ExecResult execute(code::ByteCode const* code);

   private:
    code::ByteCode const* code_{nullptr};
    code::OpCode const* ip_{nullptr};

    ExecResult run();
  };
}  // namespace fluir

#endif
