#ifndef FLUIR_EDITOR_VIEW_TOOLBAR_HPP
#define FLUIR_EDITOR_VIEW_TOOLBAR_HPP

#include <string>
#include <vector>

#include "editor/components/button.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/text_metrics.hpp"
#include "editor/input.hpp"

namespace fluir::editor {

  /** A bar of self-sizing buttons plus an optional label after the left ones. */
  struct Toolbar {
    std::vector<ButtonComp> left;  /**< Buttons aligned to the left edge, in order */
    std::vector<ButtonComp> right; /**< Buttons aligned to the right edge, in order */
    std::string label;
  };

  /** Where a toolbar's bar and label sit, in screen px. */
  struct ToolbarLayout {
    Rect bar;
    double labelX = 0.0;
  };

  /** Places `toolbar`'s buttons across a bar `width` px wide. */
  ToolbarLayout layoutToolbar(Toolbar& toolbar,
                              double width,
                              const EditorContext::Layout& layout,
                              TextMetrics& metrics);

  void drawToolbar(Renderer& renderer, const Toolbar& toolbar, const ToolbarLayout& layout, const EditorContext& ctx);

  /** Offers `event` to each button. Returns whether any button handled the event. */
  bool handleToolbar(Toolbar& toolbar, const InputEvent& event);

}  // namespace fluir::editor

#endif
