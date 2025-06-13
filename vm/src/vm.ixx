module;

#include <cstdint>
#include <vector>

export module fluir.vm;
export import fluir.byte_code;

namespace fluir {
  export enum class ExecResult { SUCCESS, ERROR };

  export class VirtualMachine {
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
  };
}  // namespace fluir
