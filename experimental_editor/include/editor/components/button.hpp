#ifndef FLUIR_EDITOR_COMPONENTS_BUTTON_HPP
#define FLUIR_EDITOR_COMPONENTS_BUTTON_HPP

#include <functional>
#include <string>

#include "editor/core/editor_context.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/text_metrics.hpp"
#include "editor/input.hpp"

namespace fluir::editor {
  struct ButtonOptions {
    std::string label;
    std::function<void()> onClick;
    std::function<bool()> enabled;
  };

  /** A generic button */
  class Button {
   public:
    explicit Button(ButtonOptions options);

    /** Places the button at the given Rect in screen pixels */
    void place(Rect rect) { rect_ = rect; }
    /** The button's rect in screen pixels */
    const Rect& rect() const { return rect_; }
    /** Returns the preferred size of the button */
    Vec2 preferredSize(double textPad, TextMetrics& metrics) const;
    /** The label of the button */
    const std::string& label() const { return options_.label; }

    /** Indicates whether the button is enabled */
    bool enabled() const;
    /** Handles an input event. Returns true if handled, false otherwise.
     * If the button is disabled, returns if it would have handled the event. */
    bool handle(const InputEvent& event);

    /** Draws the button to the given renderer, subject to the given theme.*/
    void draw(Renderer& renderer, const EditorContext::Theme& theme) const;

   private:
    ButtonOptions options_;
    Rect rect_{};
    bool armed_ = false;

    void drawCenteredLabel(Renderer& renderer, const EditorContext::Theme& theme) const;
  };

}  // namespace fluir::editor

#endif
