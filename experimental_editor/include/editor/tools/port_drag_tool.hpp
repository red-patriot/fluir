#ifndef FLUIR_EDITOR_TOOLS_PORT_DRAG_TOOL_HPP
#define FLUIR_EDITOR_TOOLS_PORT_DRAG_TOOL_HPP

#include <memory>

#include "editor/core/tree_path.hpp"
#include "editor/tools/grid_steps.hpp"
#include "editor/tools/tool.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Drag on a conditional's port slides it up/down its wall. */
  class PortDragTool : public Tool {
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
    PortRef port_;
    int startY_ = 0;
    int deltaY_ = 0;
    GridSteps steps_;
    std::unique_ptr<Transaction> edit_; /**< executed on the tree while non-null */
  };

}  // namespace fluir::editor

#endif
