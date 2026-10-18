#ifndef FLUIR_COMPILER_BACKEND_FUNC_FRAME_HPP
#define FLUIR_COMPILER_BACKEND_FUNC_FRAME_HPP

#include <cstdint>

#include <bytecode/code_chunk.hpp>

#include "compiler/utility/context.hpp"

namespace fluir::be {

  /** Returns the number of slots the given type takes up on the stack*/
  uint8_t slotsOf(const types::Type& type);

  class Scope {
   public:
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
    Scope(Scope&&) = delete;
    Scope& operator=(Scope&&) = delete;
    ~Scope();

    /** Add a new local slot, and return its slot number. If id already exists in a slot, returns its slot without
     * changes. */
    size_t addLocalSlot(ID id, const types::Type& type);
    /** Indicates if the given ID is known in the current scope. */
    bool contains(const ID& id) const;
    /** Gets an existing local slot. Asserts the given ID is already known as a local. */
    size_t getLocalSlot(const ID& id) const;
    Scope pushScope() const;

   private:
    friend class BytecodeFrame;
    explicit Scope(class BytecodeFrame* frame);

    BytecodeFrame* frame_;
    std::unordered_map<ID, size_t> slots_{};
    size_t outputCount_{0};
  };

  class BytecodeFrame {
   public:
    BytecodeFrame(Context& ctx,
                  const std::vector<WithID<types::TypeID>>& inputs,
                  const std::vector<WithID<types::TypeID>>& outputs,
                  code::Bytes& code);

    BytecodeFrame(const BytecodeFrame&) = delete;
    BytecodeFrame& operator=(const BytecodeFrame&) = delete;
    BytecodeFrame(BytecodeFrame&&) = delete;
    BytecodeFrame& operator=(BytecodeFrame&&) = delete;
    ~BytecodeFrame();

    Scope pushScope();

   private:
    friend class Scope;

    Context& ctx_;
    code::Bytes& code_;

    size_t currentSlots_{0};
    size_t maxSlots_{0};
    size_t reserveAt_{0};
    uint8_t inputCount_{0};
    uint8_t outputCount_{0};
  };
}  // namespace fluir::be

#endif
