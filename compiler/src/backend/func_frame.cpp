#include "compiler/backend/func_frame.hpp"

#include <cmath>

namespace fluir::be {
  BytecodeFrame::BytecodeFrame(Context& ctx,
                               const std::vector<WithID<types::TypeID>>& inputs,
                               const std::vector<WithID<types::TypeID>>& outputs,
                               code::Bytes& code) :
    ctx_(ctx), code_(code), inputCount_(inputs.size()), outputCount_(outputs.size()) {
    constexpr uint8_t PLACEHOLDER = 0xff;
    code_.push_back(code::RESERVE);
    code_.push_back(PLACEHOLDER);
    reserveAt_ = code_.size() - 1;
  }

  BytecodeFrame::~BytecodeFrame() {
    // Patch the reserve at the frame top to reserve the correct amount of stack space
    const auto maxLocalsAndInputs = maxSlots_ - outputCount_;
    const auto toReserve =
      maxLocalsAndInputs - inputCount_;  // Don't reserve space for inputs, they are already in place
    code_[reserveAt_] = toReserve;
    if (maxLocalsAndInputs > 0) {
      code_.push_back(code::MULTIPOP);
      code_.push_back(maxLocalsAndInputs);
    }
  }

  Scope BytecodeFrame::pushScope() { return Scope(this); }

  Scope::Scope(BytecodeFrame* frame) : frame_(frame) { }

  Scope::~Scope() {
    auto localsInScope = slots_.size();
    frame_->currentSlots_ -= localsInScope;
  }

  size_t Scope::addLocalSlot(ID id, const types::Type& type) {
    if (!slots_.contains(id)) {
      auto slot = frame_->currentSlots_;
      frame_->maxSlots_ = std::max(frame_->maxSlots_, slot + 1);  // Handle off by one due to indexing
      frame_->currentSlots_ += slotsOf(type);
      slots_.emplace(id, slot);
    }

    return slots_.at(id);
  }

  bool Scope::contains(const ID& id) const { return slots_.contains(id); }
  size_t Scope::getLocalSlot(const ID& id) const { return slots_.at(id); }
  Scope Scope::pushScope() const { return Scope(frame_); }

  uint8_t slotsOf(const types::Type& type) {
    if (type.is<types::Product>()) {
      return type.as<types::Product>()->elements();
    }
    // All scalar types take up 1 slot
    return 1;
  }

}  // namespace fluir::be
