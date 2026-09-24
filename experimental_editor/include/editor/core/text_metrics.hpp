#ifndef FLUIR_EDITOR_CORE_TEXT_METRICS_HPP
#define FLUIR_EDITOR_CORE_TEXT_METRICS_HPP

#include <cstddef>
#include <string_view>

#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** Text layout queries, in screen px. */
  class TextMetrics {
   public:
    virtual ~TextMetrics() = default;

    /** Screen-px size `text` would occupy. */
    virtual Vec2 measureText(std::string_view text) = 0;
    /** Caret index nearest `point` in `text` laid out on screen with wrapping enabled. */
    virtual std::size_t wrappedIndexAt(Rect screen, std::string_view text, double scale, Vec2 point) = 0;
    /** 1 px wide caret before `index` of `text` laid out on screen with wrapping enabled. */
    virtual Rect wrappedCaretRect(Rect screen, std::string_view text, double scale, std::size_t index) = 0;
  };

}  // namespace fluir::editor

#endif
