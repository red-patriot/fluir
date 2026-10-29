#include "compiler/backend/func_frame.hpp"

#include <cmath>

namespace fluir::be {
  BytecodeFrame::BytecodeFrame(Context& ctx,
                               const std::vector<WithID<types::TypeID>>& inputs,
                               const std::vector<WithID<types::TypeID>>& outputs,
                               code::Bytes& code) :
    ctx_(ctx),
    code_(code),
    inputCount_(static_cast<uint8_t>(inputs.size())),
    outputCount_(static_cast<uint8_t>(outputs.size())) {
    constexpr uint8_t PLACEHOLDER = 0xff;
    code_.push_back(code::RESERVE);
    code_.push_back(PLACEHOLDER);
    reserveAt_ = code_.size() - 1;
  }

  BytecodeFrame::~BytecodeFrame() {
    // Patch the reserve at the frame top to reserve the correct amount of stack space
    const auto maxLocalsAndInputs = static_cast<uint8_t>(maxSlots_ - outputCount_);
    const auto toReserve = static_cast<uint8_t>(
      maxLocalsAndInputs - inputCount_);  // Don't reserve space for inputs, they are already in place
    code_[reserveAt_] = toReserve;
    if (maxLocalsAndInputs > 0) {
      code_.push_back(code::MULTIPOP);
      code_.push_back(maxLocalsAndInputs);
    }
  }

  Scope BytecodeFrame::pushScope() { return Scope(this); }

  Scope::Scope(BytecodeFrame* frame) : frame_(frame) { }

  Scope::~Scope() {
    // The frame will reserve once up front the max number of slots needed by any of its scope,
    // so recalc the max before popping this scope's slots
    frame_->maxSlots_ = std::max(frame_->maxSlots_, frame_->currentSlots_);
    auto localsInScope = slots_.size();
    frame_->currentSlots_ -= localsInScope;
  }

  size_t Scope::addLocalSlot(ID id, const types::Type& type) {
    if (!slots_.contains(id)) {
      auto baseSlot = frame_->currentSlots_;
      auto slots = slotsOf(type);
      frame_->currentSlots_ += slots;
      slots_.emplace(id, baseSlot);
    }

    return slots_.at(id);
  }

  bool Scope::contains(const ID& id) const { return slots_.contains(id); }
  size_t Scope::getLocalSlot(const ID& id) const { return slots_.at(id); }
  Scope Scope::pushScope() const { return Scope(frame_); }

  uint8_t slotsOf(const types::Type& type) {
    if (type.is<types::Product>()) {
      return static_cast<uint8_t>(type.as<types::Product>()->elements());
    }
    // All scalar types take up 1 slot
    return 1;
  }

}  // namespace fluir::be
