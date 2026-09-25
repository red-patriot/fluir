#include "editor/tools/text_edit_tool.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "editor/core/fields.hpp"
#include "editor/core/node_access.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/transaction.hpp"
#include "editor/view/draw/comment.hpp"
#include "editor/view/draw/function.hpp"
#include "editor/view/node_view.hpp"

// Edits any text field: where it sits comes from the view's `labels`, its value and edit from `core/fields`.

namespace fluir::editor {
  namespace {

    // What a press at `world` opens, or nullopt.
    std::optional<TextEditTool::Target> targetAt(const et::ParseTree& tree, std::span<const Box> boxes, Vec2 world) {
      const Box* label = labelAt(boxes, world);
      if (!label || isChoice(label->field->kind) || !fields::read(tree, label->path, *label->field)) {
        return std::nullopt;
      }
      return TextEditTool::Target{label->path, *label->field};
    }

    // Where the target's text sits and the clip it draws under; the draft covers it while drawn.
    struct Label {
      Rect rect;
      std::optional<Rect> clip;
    };

    std::optional<Label> labelRect(std::span<const Box> boxes, const TextEditTool::Target& target) {
      const auto it = std::ranges::find_if(boxes, [&](const Box& box) {
        return box.part == Part::Label && box.path == target.path && box.field == target.field;
      });
      return it == boxes.end() ? std::nullopt : std::optional{Label{it->world, it->clip}};
    }

    // A function's name or parameter sits on header chrome; everything else on its node.
    Color coverColor(const et::ParseTree& tree, const FullID& path, const EditorContext::Theme& theme) {
      if (const et::FunctionDecl* fn = functionAt(tree, path)) {
        return draw::color(*fn, theme);
      }
      if (commentAt(tree, path) != nullptr) {
        return theme.commentNode;
      }
      const et::Node* node = nodeAt(tree, path);
      return node == nullptr ? theme.background : nodeColor(*node, theme);
    }

    Vec2 textOrigin(const Rect& label, const EditorContext::Layout& layout) {
      return {label.x + layout.textPad, label.y + layout.textPad};
    }

  }  // namespace

  bool TextEditTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    // An undo or delete may have removed what the draft edits.
    if (field_ && !fields::read(state.editor.tree(), target_.path, target_.field)) {
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
    const et::ParseTree& tree = state.editor.tree();
    const Vec2 world = state.view.screenToWorld(event.pos);
    // A terminal press starts a conduit, so it only closes the draft.
    if (terminalAt(tree, boxes, world, state.ctx.layout)) {
      field_.reset();
      return;
    }
    const std::optional<Target> target = targetAt(tree, boxes, world);
    const std::optional<Label> label = target ? labelRect(boxes, *target) : std::nullopt;
    if (!label) {
      field_.reset();
      return;
    }
    const bool wrapped = commentAt(tree, target->path) != nullptr;
    const double dx = world.x - textOrigin(label->rect, state.ctx.layout).x;
    const auto caretAt = [&](const std::string& text) -> std::size_t {
      if (!wrapped) {
        return TextField::indexAt(text, dx);
      }
      if (state.text == nullptr) {
        return text.size();
      }
      const Rect local = commentTextRect(label->rect, state.ctx.layout);
      return state.text->wrappedIndexAt(state.view.toScreen(local), text, state.view.scale, event.pos);
    };
    if (field_ && target_ == *target) {
      field_->setCaret(caretAt(field_->text()));
      return;
    }
    const std::string text = *fields::read(tree, target->path, target->field);
    target_ = *target;
    field_.emplace(text, caretAt(text));
  }

  bool TextEditTool::onKey(InputEvent::Key key, EditorState& state) {
    switch (key) {
      case InputEvent::Key::Return:
        {
          std::optional<std::unique_ptr<Transaction>> edit =
            fields::write(state.editor.tree(), target_.path, target_.field, field_->text());
          if (!edit) {
            field_->reject();
            return true;
          }
          if (*edit) {
            state.editor.apply(std::move(*edit));
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

  // Covers the committed label with the owner's own chrome, then draws the draft.
  void TextEditTool::draw(const Subview& view, const EditorState& state, std::span<const Box> boxes) const {
    if (!field_) {
      return;
    }
    const et::ParseTree& tree = state.editor.tree();
    const std::optional<Label> label = labelRect(boxes, target_);
    if (!label) {
      return;
    }
    Renderer& r = view.renderer();
    if (label->clip) {
      r.pushClip(view.toScreen(*label->clip));
    }
    r.fillRect(view.toScreen(label->rect), coverColor(tree, target_.path, state.ctx.theme));
    r.drawRect(view.toScreen(label->rect), state.ctx.theme.border);
    if (commentAt(tree, target_.path) != nullptr) {
      field_->drawWrapped(view, state.ctx, commentTextRect(label->rect, state.ctx.layout));
    } else {
      field_->draw(view, state.ctx, textOrigin(label->rect, state.ctx.layout));
    }
    if (field_->invalid()) {
      r.drawRect(view.toScreen(label->rect), state.ctx.theme.error);
    }
    if (label->clip) {
      r.popClip();
    }
  }

}  // namespace fluir::editor
