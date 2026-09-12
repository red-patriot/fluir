#ifndef FLUIR_EDITOR_TRANSACTION_RESIZE_HPP
#define FLUIR_EDITOR_TRANSACTION_RESIZE_HPP

#include <utility>

#include "compiler/models/id.hpp"
#include "editor/actors/scene.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Smallest and largest size an edit may leave an actor at, in grid units. */
  constexpr int MIN_SIZE = 4;
  constexpr int MAX_SIZE = 1000;

  /** Resizes an actor to (width, height). Self-inverting: the swap is its own reverse. */
  class ResizeTransaction : public Transaction {
    /** `size` clamped to the range an edit may produce. */
    static int clampSize(int size);

   public:
    ResizeTransaction(fluir::FullID id, int width, int height) :
      id_(std::move(id)), width_(clampSize(width)), height_(clampSize(height)) { }

    bool execute(GraphScene& scene) override;
    bool unexecute(GraphScene& scene) override { return execute(scene); }

   private:
    fluir::FullID id_;
    int width_;
    int height_;
  };

}  // namespace fluir::editor

#endif
