#pragma once

#include <memory>

#include "compiler/models/location.hpp"
#include "editor/tools/tool.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Left-drag on a grip moves or resizes its owner live, recording one edit on release. */
  class DragTool : public Tool {
   public:
    bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) override;
    bool capturing() const override { return active_; }
    void cancel(EditorState& state) override;

   private:
    /** Swaps the live edit for one aimed at the current grid delta. */
    void reaim(EditorState& state);
    void reset();

    bool active_ = false;
    FullID path_;
    Part part_ = Part::Body;
    FlowGraphLocation start_{};
    Vec2 lastWorld_;
    Vec2 remainder_;
    Vec2i delta_;
    std::unique_ptr<Transaction> edit_; /**< executed on the tree while non-null */
  };

}  // namespace fluir::editor
