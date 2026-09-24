#ifndef FLUIR_EDITOR_COMPONENTS_MENU_HPP
#define FLUIR_EDITOR_COMPONENTS_MENU_HPP

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

  /** Rows `labels` under `anchor`, flipping above past `bounds`' bottom. Measures with `text`; null estimates GLYPH_PX.
   */
  MenuLayout layoutMenu(std::span<const std::string> labels,
                        Rect anchor,
                        Rect bounds,
                        const EditorContext::Layout& layout,
                        TextMetrics* text);

  /** Rows past `enabled`'s end are enabled; a disabled row is greyed and never highlighted. */
  void drawMenu(Renderer& renderer,
                std::span<const std::string> labels,
                const MenuLayout& layout,
                std::optional<std::size_t> hovered,
                const EditorContext& ctx,
                const std::vector<bool>& enabled = {});

  /** Whether row `i` is enabled; rows past `enabled`'s end are. */
  bool menuRowEnabled(const std::vector<bool>& enabled, std::size_t i);

  /** Index of the row under `screen`, or nullopt. */
  std::optional<std::size_t> menuItemAt(const MenuLayout& layout, Vec2 screen);

}  // namespace fluir::editor

#endif
