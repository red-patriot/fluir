#include "editor/tools/conduit_tool.hpp"

#include <memory>
#include <optional>

#include "editor/core/renderer.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/add_conduit.hpp"

namespace fluir::editor {

  bool ConduitTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    switch (event.type) {
      case InputEvent::Type::MouseDown:
        {
          if (active_ || event.button != InputEvent::Button::Left) {
            return false;
          }
          const Vec2 world = state.view.screenToWorld(event.pos);
          const std::optional<TerminalHit> hit = terminalAt(state.editor.tree(), boxes, world, state.ctx.layout);
          if (!hit) {
            return false;
          }
          active_ = true;
          from_ = *hit;
          cursor_ = world;
          return true;
        }

      case InputEvent::Type::MouseMove:
        if (!active_) {
          return false;
        }
        cursor_ = state.view.screenToWorld(event.pos);
        return true;

      case InputEvent::Type::MouseUp:
        {
          if (!active_ || event.button != InputEvent::Button::Left) {
            return false;
          }
          active_ = false;
          const std::optional<TerminalHit> to =
            terminalAt(state.editor.tree(), boxes, state.view.screenToWorld(event.pos), state.ctx.layout);
          if (!to || to->output == from_.output || parentOf(to->path) != parentOf(from_.path)) {
            return true;
          }
          // Either end may start the drag; the conduit always runs output to input.
          const TerminalHit& source = from_.output ? from_ : *to;
          const TerminalHit& target = from_.output ? *to : from_;
          const FullID parent = parentOf(source.path);
          state.editor.apply(std::make_unique<AddConduit>(parent,
                                                          state.editor.generateID(parent),
                                                          AddConduit::Endpoint{source.path.back(), source.index},
                                                          AddConduit::Endpoint{target.path.back(), target.index}));
          return true;
        }

      case InputEvent::Type::KeyDown:
        if (!active_ || event.key != InputEvent::Key::Escape) {
          return false;
        }
        active_ = false;
        return true;

      default:
        return false;
    }
  }

  void ConduitTool::draw(const Subview& view, const EditorState& state, std::span<const Box>) const {
    if (active_) {
      view.renderer().drawLine(view.toScreen(from_.anchor), view.toScreen(cursor_), state.ctx.theme.conduit);
    }
  }

}  // namespace fluir::editor
