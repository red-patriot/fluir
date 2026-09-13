#include "editor/tools/pan_zoom_tool.hpp"

#include <cmath>

namespace fluir::editor {

  bool PanZoomTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box>) {
    switch (event.type) {
      case InputEvent::Type::MouseDown:
        if (event.button != InputEvent::Button::Middle) {
          return false;
        }
        panning_ = true;
        last_ = event.pos;
        return true;

      case InputEvent::Type::MouseMove:
        if (!panning_) {
          return false;
        }
        state.view.pan = state.view.pan + (event.pos - last_);
        last_ = event.pos;
        return true;

      case InputEvent::Type::MouseUp:
        if (!panning_ || event.button != InputEvent::Button::Middle) {
          return false;
        }
        panning_ = false;
        return true;

      case InputEvent::Type::Wheel:
        {
          const Viewport old = state.view;
          state.view.zoomAbout(event.pos, std::pow(state.ctx.zoom.wheelStep, event.wheel.y));
          if (state.view.scale < state.ctx.zoom.min || state.view.scale > state.ctx.zoom.max) {
            state.view = old;
          }
          return true;
        }

      default:
        return false;
    }
  }

}  // namespace fluir::editor
