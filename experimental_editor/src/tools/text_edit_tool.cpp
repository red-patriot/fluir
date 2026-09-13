#include "editor/tools/text_edit_tool.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <variant>

#include "editor/core/literal_text.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/set_constant_value.hpp"
#include "editor/view/node_view.hpp"

namespace fluir::editor {
  namespace {

    const pt::Constant* constantAt(const pt::ParseTree& tree, const FullID& path) {
      const pt::Node* node = nodeAt(tree, path);
      return node == nullptr ? nullptr : std::get_if<pt::Constant>(node);
    }

    Vec2 textOrigin(const Rect& body, const EditorContext::Layout& layout) {
      return {body.x + layout.textPad, body.y + layout.textPad};
    }

  }  // namespace

  bool TextEditTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    // An undo or delete may have removed the constant under the draft.
    if (field_ && constantAt(state.editor.tree(), path_) == nullptr) {
      field_.reset();
    }
    switch (event.type) {
      case InputEvent::Type::MouseDown:
        if (event.button == InputEvent::Button::Left) {
          onPress(event, state, boxes);
        }
        return false;  // tracked, never consumed
      case InputEvent::Type::KeyDown:
        return field_ && event.key && onKey(*event.key, state);
      case InputEvent::Type::TextInput:
        if (!field_) {
          return false;
        }
        field_->insert(event.text);
        return true;
      default:
        return false;
    }
  }

  void TextEditTool::onPress(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    const Vec2 world = state.view.screenToWorld(event.pos);
    const Box* hit = hitAt(boxes, world);
    const pt::Constant* constant =
      hit != nullptr && hit->part == Part::Body ? constantAt(state.editor.tree(), hit->path) : nullptr;
    if (constant == nullptr || !isEditableLiteral(constant->value)) {
      field_.reset();
      return;
    }
    // Glyphs are fixed screen px, so the caret offset is measured on screen.
    const double dx = (world.x - textOrigin(hit->world, state.ctx.layout).x) * state.view.scale;
    if (field_ && path_ == hit->path) {
      field_->setCaretFromOffset(dx);
      return;
    }
    const std::string text = renderLiteral(constant->value);
    path_ = hit->path;
    field_.emplace(text, TextField::indexAt(text, dx));
  }

  bool TextEditTool::onKey(InputEvent::Key key, EditorState& state) {
    switch (key) {
      case InputEvent::Key::Return:
        {
          const pt::Literal& value = constantAt(state.editor.tree(), path_)->value;
          const std::optional<pt::Literal> parsed = parseLiteralLike(value, field_->text());
          if (!parsed) {
            field_->reject();
            return true;
          }
          if (*parsed != value) {
            state.editor.apply(std::make_unique<SetConstantValueTransaction>(path_, *parsed));
          }
          field_.reset();
          return true;
        }
      case InputEvent::Key::Escape:
        field_.reset();
        return true;
      default:
        // An open draft owns every key: unhandled ones still arrive as its TextInput.
        field_->onKey(key);
        return true;
    }
  }

  // Covers the committed value with the node's own chrome, then draws the draft.
  void TextEditTool::draw(const Subview& view, const EditorState& state, std::span<const Box> boxes) const {
    if (!field_) {
      return;
    }
    const auto body =
      std::ranges::find_if(boxes, [this](const Box& b) { return b.part == Part::Body && b.path == path_; });
    const pt::Node* node = nodeAt(state.editor.tree(), path_);
    if (body == boxes.end() || node == nullptr) {
      return;
    }
    Renderer& r = view.renderer();
    if (body->clip) {
      r.pushClip(view.toScreen(*body->clip));
    }
    r.fillRect(view.toScreen(body->world), nodeColor(*node, state.ctx.theme));
    r.drawRect(view.toScreen(body->world), state.ctx.theme.border);
    field_->draw(view, state.ctx, textOrigin(body->world, state.ctx.layout));
    if (field_->invalid()) {
      r.drawRect(view.toScreen(body->world), state.ctx.theme.error);
    }
    if (body->clip) {
      r.popClip();
    }
  }

}  // namespace fluir::editor
