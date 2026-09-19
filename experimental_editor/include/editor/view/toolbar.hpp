#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/renderer.hpp"

namespace fluir::editor {

  /** A labeled screen-space button. */
  struct Button {
    enum class Align { Left, Right };

    std::string label;
    std::function<void()> onClick;
    std::function<bool()> enabled = {};
    Align align = Align::Left;
  };

  /** A bar of self-sizing buttons plus an optional trailing label. */
  struct Toolbar {
    std::vector<Button> buttons;
    std::string label;
  };

  /** Where a toolbar's pieces sit, in screen px. `buttons` parallels `Toolbar::buttons`. */
  struct ToolbarLayout {
    Rect bar;
    std::vector<Rect> buttons;
    double labelX = 0.0;
  };

  /** Rows `toolbar`'s buttons across a bar `width` px wide. */
  ToolbarLayout layoutToolbar(const Toolbar& toolbar,
                              double width,
                              const EditorContext::Layout& layout,
                              Renderer& renderer);

  void drawToolbar(Renderer& renderer, const Toolbar& toolbar, const ToolbarLayout& layout, const EditorContext& ctx);

  void drawButton(Renderer& renderer, const Button& button, Rect rect, const EditorContext& ctx);

  /** Index of the button under `screen`, or nullopt. */
  std::optional<std::size_t> buttonAt(const ToolbarLayout& layout, Vec2 screen);

}  // namespace fluir::editor
