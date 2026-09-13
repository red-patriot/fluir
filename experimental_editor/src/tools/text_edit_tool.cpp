#include "editor/tools/text_edit_tool.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "editor/core/identifier.hpp"
#include "editor/core/literal_text.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/edit_call_node.hpp"
#include "editor/transaction/rename.hpp"
#include "editor/transaction/set_constant_value.hpp"
#include "editor/transaction/transaction.hpp"
#include "editor/view/node_view.hpp"

// Editable kinds: a constant's literal, a call's target and a function's name. A new kind adds a branch to
// `labelRect`, `draftText` and `commit` below.

namespace fluir::editor {
  namespace {

    const pt::Constant* constantAt(const pt::ParseTree& tree, const FullID& path) {
      const pt::Node* node = nodeAt(tree, path);
      return node == nullptr ? nullptr : std::get_if<pt::Constant>(node);
    }

    const pt::Call* callAt(const pt::ParseTree& tree, const FullID& path) {
      const pt::Node* node = nodeAt(tree, path);
      return node == nullptr ? nullptr : std::get_if<pt::Call>(node);
    }

    // Where the editable text sits inside a body box: the draft opens on a press there and covers it while drawn.
    std::optional<Rect> labelRect(const pt::ParseTree& tree, const Box& body, const EditorContext::Layout& layout) {
      if (functionAt(tree, body.path) != nullptr) {
        return Rect{body.world.x, body.world.y, body.world.w, layout.headerH()};
      }
      if (callAt(tree, body.path) != nullptr) {
        return Rect{body.world.x, body.world.y, body.world.w, std::min(body.world.h, layout.railStep())};
      }
      if (const pt::Constant* constant = constantAt(tree, body.path); constant && isEditableLiteral(constant->value)) {
        return body.world;
      }
      return std::nullopt;
    }

    std::optional<std::string> draftText(const pt::ParseTree& tree, const FullID& path) {
      if (const pt::FunctionDecl* fn = functionAt(tree, path)) {
        return fn->name;
      }
      if (const pt::Call* call = callAt(tree, path)) {
        return call->target;
      }
      if (const pt::Constant* constant = constantAt(tree, path); constant && isEditableLiteral(constant->value)) {
        return renderLiteral(constant->value);
      }
      return std::nullopt;
    }

    // The edit `text` commits: nullopt rejects the draft, a null edit means the value is unchanged.
    std::optional<std::unique_ptr<Transaction>> commit(const pt::ParseTree& tree,
                                                       const FullID& path,
                                                       const std::string& text) {
      if (const pt::FunctionDecl* fn = functionAt(tree, path)) {
        if (!isValidIdentifier(text)) {
          return std::nullopt;
        }
        return text == fn->name ? nullptr : std::make_unique<RenameTransaction>(path, text);
      }
      if (const pt::Call* call = callAt(tree, path)) {
        if (!isValidIdentifier(text)) {
          return std::nullopt;
        }
        return text == call->target ? nullptr : std::make_unique<EditCallNodeTransaction>(path, text);
      }
      const pt::Literal& value = constantAt(tree, path)->value;
      const std::optional<pt::Literal> parsed = tryParseLiteral(value, text);
      if (!parsed) {
        return std::nullopt;
      }
      return *parsed == value ? nullptr : std::make_unique<SetConstantValueTransaction>(path, *parsed);
    }

    Color coverColor(const pt::ParseTree& tree, const FullID& path, const EditorContext::Theme& theme) {
      if (functionAt(tree, path) != nullptr) {
        return theme.funcDeclHeader;
      }
      const pt::Node* node = nodeAt(tree, path);
      return node == nullptr ? theme.background : nodeColor(*node, theme);
    }

    Vec2 textOrigin(const Rect& label, const EditorContext::Layout& layout) {
      return {label.x + layout.textPad, label.y + layout.textPad};
    }

  }  // namespace

  bool TextEditTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    // An undo or delete may have removed what the draft edits.
    if (field_ && !draftText(state.editor.tree(), path_)) {
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
    const std::optional<Rect> label =
      hit != nullptr && hit->part == Part::Body ? labelRect(state.editor.tree(), *hit, state.ctx.layout) : std::nullopt;
    if (!label || !label->contains(world)) {
      field_.reset();
      return;
    }
    // Glyphs are fixed screen px, so the caret offset is measured on screen.
    const double dx = (world.x - textOrigin(*label, state.ctx.layout).x) * state.view.scale;
    if (field_ && path_ == hit->path) {
      field_->setCaretFromOffset(dx);
      return;
    }
    const std::string text = *draftText(state.editor.tree(), hit->path);
    path_ = hit->path;
    field_.emplace(text, TextField::indexAt(text, dx));
  }

  bool TextEditTool::onKey(InputEvent::Key key, EditorState& state) {
    switch (key) {
      case InputEvent::Key::Return:
        {
          std::optional<std::unique_ptr<Transaction>> edit = commit(state.editor.tree(), path_, field_->text());
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
    const pt::ParseTree& tree = state.editor.tree();
    const auto body =
      std::ranges::find_if(boxes, [this](const Box& b) { return b.part == Part::Body && b.path == path_; });
    if (body == boxes.end()) {
      return;
    }
    const std::optional<Rect> label = labelRect(tree, *body, state.ctx.layout);
    if (!label) {
      return;
    }
    Renderer& r = view.renderer();
    if (body->clip) {
      r.pushClip(view.toScreen(*body->clip));
    }
    r.fillRect(view.toScreen(*label), coverColor(tree, path_, state.ctx.theme));
    r.drawRect(view.toScreen(*label), state.ctx.theme.border);
    field_->draw(view, state.ctx, textOrigin(*label, state.ctx.layout));
    if (field_->invalid()) {
      r.drawRect(view.toScreen(*label), state.ctx.theme.error);
    }
    if (body->clip) {
      r.popClip();
    }
  }

}  // namespace fluir::editor
