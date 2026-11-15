#ifndef FLUIR_VM_MACHINE_VM_HPP
#define FLUIR_VM_MACHINE_VM_HPP

#include <array>
#include <memory>
#include <span>
#include <vector>

#include "vm/code/byte_code.hpp"
#include "vm/code/native_func.hpp"
#include "vm/machine/call_frame.hpp"

namespace fluir {
  enum class ExecResult { SUCCESS = 0, ERROR, ERROR_DIVIDE_BY_ZERO };

  class VirtualMachine {
    static constexpr size_t FUNCTION_DEPTH = 512;
    static constexpr size_t FRAME_LIMIT = 256;
    static constexpr size_t STACK_LIMIT = FUNCTION_DEPTH * FRAME_LIMIT;

   public:
    using Stack = std::array<code::Value, STACK_LIMIT>;

    VirtualMachine() = default;
    explicit VirtualMachine(NativeFunctionsMap natives);
    VirtualMachine(const VirtualMachine&) = delete;
    VirtualMachine& operator=(const VirtualMachine&) = delete;
    VirtualMachine(VirtualMachine&&) = delete;
    VirtualMachine& operator=(VirtualMachine&&) = delete;
    ~VirtualMachine() = default;

    ExecResult execute(code::ByteCode const* code);

    std::span<const code::Value> viewStack() const { return {currentFrame_->basePtr, currentFrame_->stackEnd}; }

   private:
    code::ByteCode const* code_{nullptr};
    std::vector<CallFrame> frames_;
    std::uint8_t const* ip_{nullptr};
    CallFrame* currentFrame_{nullptr};
    std::unique_ptr<Stack> stack_;
    code::Chunk flStartup_;
    NativeFunctionsMap natives_{};

    /** Initializes the VM state to begin running the bytecode */
    void init();
    /** Runs the bytecode until it finishes */
    ExecResult run();

    /** Loads the internal startup function */
    void createFlStartup(size_t mainIndex);
    size_t stackSize() const { return currentFrame_->stackEnd - stack_->data(); }
    code::Value& stackTop();
    code::Value stackPopTop();
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

    std::uint8_t readByte();
    std::uint64_t readQuadWord();
    void initCall(code::Chunk const* callee, code::Value* basePtr);
  };

}  // namespace fluir

#endif
