#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/renderer.hpp"

namespace fluir::editor {

  /** Where a menu's pieces sit, in screen px. `items` parallels its labels. */
  struct MenuLayout {
    Rect frame;
    std::vector<Rect> items;
  };

  /** Rows `labels` under `anchor`, flipping above it when they would pass `bounds`' bottom. */
  MenuLayout layoutMenu(std::span<const std::string> labels,
                        Rect anchor,
                        Rect bounds,
                        const EditorContext::Layout& layout);

  void drawMenu(Renderer& renderer,
                std::span<const std::string> labels,
                const MenuLayout& layout,
                std::optional<std::size_t> hovered,
                const EditorContext& ctx);

  /** Index of the row under `screen`, or nullopt. */
  std::optional<std::size_t> menuItemAt(const MenuLayout& layout, Vec2 screen);

}  // namespace fluir::editor
