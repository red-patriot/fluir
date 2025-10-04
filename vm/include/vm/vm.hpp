#ifndef FLUIR_VM_CM_HPP
#define FLUIR_VM_CM_HPP

#include <bytecode/byte_code.hpp>

namespace fluir {
  enum class ExecResult { SUCCESS, ERROR };

  class VirtualMachine {
   public:
    using Stack = std::vector<code::Value>;

    VirtualMachine() = default;
    VirtualMachine(const VirtualMachine&) = delete;
    VirtualMachine& operator=(const VirtualMachine&) = delete;
    VirtualMachine(VirtualMachine&&) = delete;
    VirtualMachine& operator=(VirtualMachine&&) = delete;
    ~VirtualMachine() = default;

    ExecResult execute(code::ByteCode const* code);

    const Stack& viewStack() const { return stack_; }

   private:
    code::ByteCode const* code_{nullptr};
    code::Chunk const* current_{nullptr};
    std::uint8_t const* ip_{nullptr};
    Stack stack_;

    ExecResult run();

    template <typename Op>
    void floatBinary();
    template <typename Op>
    void floatUnary();
    template <typename Op>
    void intBinary();
    template <typename Op>
    void intUnary();
    template <typename Op>
    void uintBinary();
    template <typename Op>
    void uintUnary();
  };
}  // namespace fluir

#endif
