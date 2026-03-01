#ifndef FLUIR_VM_MACHINE_VM_HPP
#define FLUIR_VM_MACHINE_VM_HPP

#include <memory>
#include <span>

#include <bytecode/byte_code.hpp>

namespace fluir {
  enum class ExecResult { SUCCESS = 0, ERROR, ERROR_DIVIDE_BY_ZERO };

  class VirtualMachine {
    static constexpr size_t FUNCTION_DEPTH = 512;
    static constexpr size_t FRAME_LIMIT = 256;
    static constexpr size_t STACK_LIMIT = FUNCTION_DEPTH * FRAME_LIMIT;

   public:
    using Stack = std::array<code::Value, STACK_LIMIT>;

    VirtualMachine() = default;
    VirtualMachine(const VirtualMachine&) = delete;
    VirtualMachine& operator=(const VirtualMachine&) = delete;
    VirtualMachine(VirtualMachine&&) = delete;
    VirtualMachine& operator=(VirtualMachine&&) = delete;
    ~VirtualMachine() = default;

    ExecResult execute(code::ByteCode const* code);

    std::span<const code::Value> viewStack() const { return {stackBegin_, stackEnd_}; }

   private:
    code::ByteCode const* code_{nullptr};
    code::Chunk const* current_{nullptr};
    std::uint8_t const* ip_{nullptr};
    std::unique_ptr<Stack> stack_;
    code::Value* stackBegin_{nullptr};
    code::Value* stackEnd_{nullptr};

    ExecResult run();

    size_t stackSize() { return stackEnd_ - stackBegin_; }
    code::Value& stackTop();
    void popStack();
    void pushStack(code::Value value);

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
