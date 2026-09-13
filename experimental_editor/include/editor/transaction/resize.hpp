#ifndef FLUIR_EDITOR_TRANSACTION_RESIZE_HPP
#define FLUIR_EDITOR_TRANSACTION_RESIZE_HPP

#include <utility>

#include "compiler/models/id.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Resizes whatever `path` names to (width, height). Self-inverting: the swap is its own reverse. */
  class ResizeTransaction : public Transaction {
   public:
    ResizeTransaction(fluir::FullID path, int width, int height) :
      path_(std::move(path)), width_(width), height_(height) { }

    bool execute(pt::ParseTree& tree) override;
    bool unexecute(pt::ParseTree& tree) override { return execute(tree); }

   private:
    fluir::FullID path_;
    int width_;
    int height_;
  };

}  // namespace fluir::editor

#endif
