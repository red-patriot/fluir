#pragma once

#include <functional>
#include <string>
#include <vector>

#include "editor/components/button_actor.hpp"
#include "editor/components/container_actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/renderer.hpp"

namespace fluir::editor {

  /** A bar of self-sizing buttons plus an optional label. */
  class ToolbarActor : public ContainerActor {
   public:
    enum class Align { Left, Right };

    ToolbarActor(Rect bounds, Renderer& renderer) : ContainerActor(bounds), renderer_(renderer) { }

    ButtonActor& add(std::string label, std::function<void()> action, Align align = Align::Left);

    /** Trailing text drawn after the left-aligned buttons, may be empty */
    void setLabel(std::string label) { label_ = std::move(label); }

    /** Spans `width` screen px, then rows the buttons within it. */
    void resize(const EditorContext& ctx, double width);

    void layout(const EditorContext& ctx) override;

   protected:
    void drawSelf(const Subview& view, const EditorContext& ctx) const override;

   private:
    struct Entry {
      ButtonActor* button;
      Align align;
    };

    /** A button wide enough for its label plus a margin either side. */
    double buttonWidth(const EditorContext& ctx, const ButtonActor& button) const;

    Renderer& renderer_;
    std::vector<Entry> entries_;
    std::string label_;
    double labelX_ = 0.0;
  };

}  // namespace fluir::editor
