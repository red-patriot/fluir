#pragma once

#include "editor/core/editor_context.hpp"
#include "editor/input.hpp"

namespace fluir::editor {

  class Renderer;
  struct EditorState;

  /** A transient overlay the page's PopupTool hosts over the graph. Openers set `EditorState::popup`. */
  class Popup {
   public:
    virtual ~Popup() = default;

    /** Gets every event while open; false closes it. */
    virtual bool onEvent(const InputEvent& event, EditorState& state) = 0;

    /** Draws in screen px. */
    virtual void draw(Renderer& renderer, const EditorContext& ctx) const = 0;
  };

}  // namespace fluir::editor
