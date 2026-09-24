#include "editor/tools/text_edit_tool.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "editor/core/identifier.hpp"
#include "editor/core/literal_text.hpp"
#include "editor/core/node_access.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/edit_call_argument.hpp"
#include "editor/transaction/edit_call_node.hpp"
#include "editor/transaction/edit_comment.hpp"
#include "editor/transaction/rename.hpp"
#include "editor/transaction/set_constant_value.hpp"
#include "editor/transaction/transaction.hpp"
#include "editor/transaction/update_func_param.hpp"
#include "editor/view/draw/comment.hpp"
#include "editor/view/draw/function.hpp"
#include "editor/view/node_view.hpp"

// Editable kinds: a constant's literal, a call's target and argument names, a function's name and parameter names,
// and a comment's text (any text, drawn wrapped). Where each sits comes from the view's `labels`; a new kind adds a
// branch to `draftText` and `commit` below.

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

    const pt::FunctionDecl::Parameter* paramAt(const pt::FunctionDecl& fn, int index) {
      if (!fn.input) {
        return nullptr;
      }
      const auto it = std::ranges::find(fn.input->parameters, index, &pt::FunctionDecl::Parameter::index);
      return it == fn.input->parameters.end() ? nullptr : &*it;
    }

    const pt::Call::Argument* argumentAt(const pt::Call& call, int index) {
      const auto it = std::ranges::find(call.arguments, index, &pt::Call::Argument::index);
      return it == call.arguments.end() ? nullptr : &*it;
    }

    // The argument or parameter a target names, if any.
    std::optional<int> indexOf(const TextEditTool::Target& target) {
      const Field::Kind kind = target.field.kind;
      return kind == Field::Kind::Arg || kind == Field::Kind::ParamName ? std::optional{target.field.index} :
                                                                          std::nullopt;
    }

    // Whether `box` shows labels of `owner`: its body, or one of its function's rails.
    bool showsLabelsOf(const Box& box, const FullID& owner) {
      return (box.part == Part::Body && box.path == owner) || (box.part == Part::Rail && parentOf(box.path) == owner);
    }

    std::vector<FieldLabel> labelsOf(const pt::ParseTree& tree, const Box& box, const EditorContext::Layout& layout) {
      if (box.part == Part::Rail) {
        const pt::FunctionDecl* fn = functionAt(tree, parentOf(box.path));
        return fn == nullptr ? std::vector<FieldLabel>{} : draw::labels(*fn, box.path.back(), box.world, layout);
      }
      if (const pt::FunctionDecl* fn = functionAt(tree, box.path)) {
        return draw::labels(*fn, box.world, layout);
      }
      if (const pt::Comment* comment = commentAt(tree, box.path)) {
        return draw::labels(*comment, box.world, layout);
      }
      const pt::Node* node = nodeAt(tree, box.path);
      return node == nullptr ? std::vector<FieldLabel>{} : nodeLabels(*node, box.world, layout);
    }

    // Operators and bools have their own tools.
    bool isText(Field::Kind kind) { return kind != Field::Kind::Operator && kind != Field::Kind::Bool; }

    // What a press at `world` opens, or nullopt.
    std::optional<TextEditTool::Target> targetAt(const pt::ParseTree& tree,
                                                 std::span<const Box> boxes,
                                                 Vec2 world,
                                                 const EditorContext::Layout& layout) {
      const Box* hit = hitAt(boxes, world);
      if (hit == nullptr || hit->part != Part::Body) {
        return std::nullopt;
      }
      for (const Box& box : boxes) {
        if (!showsLabelsOf(box, hit->path) || (box.clip && !box.clip->contains(world))) {
          continue;
        }
        for (const FieldLabel& label : labelsOf(tree, box, layout)) {
          if (isText(label.field.kind) && label.rect.contains(world)) {
            return TextEditTool::Target{hit->path, label.field};
          }
        }
      }
      return std::nullopt;
    }

    // Where the target's text sits and the clip it draws under; the draft covers it while drawn.
    struct Label {
      Rect rect;
      std::optional<Rect> clip;
    };

    std::optional<Label> labelRect(const pt::ParseTree& tree,
                                   std::span<const Box> boxes,
                                   const TextEditTool::Target& target,
                                   const EditorContext::Layout& layout) {
      for (const Box& box : boxes) {
        if (!showsLabelsOf(box, target.path)) {
          continue;
        }
        const std::vector<FieldLabel> labels = labelsOf(tree, box, layout);
        const auto it = std::ranges::find(labels, target.field, &FieldLabel::field);
        if (it != labels.end()) {
          return Label{it->rect, box.clip};
        }
      }
      return std::nullopt;
    }

    std::optional<std::string> draftText(const pt::ParseTree& tree, const TextEditTool::Target& target) {
      if (const pt::Comment* comment = commentAt(tree, target.path)) {
        return comment->text;
      }
      const std::optional<int> index = indexOf(target);
      if (const pt::FunctionDecl* fn = functionAt(tree, target.path)) {
        if (!index) {
          return fn->name;
        }
        const pt::FunctionDecl::Parameter* param = paramAt(*fn, *index);
        return param == nullptr ? std::nullopt : std::optional{param->name};
      }
      if (const pt::Call* call = callAt(tree, target.path)) {
        if (!index) {
          return call->target;
        }
        const pt::Call::Argument* arg = argumentAt(*call, *index);
        return arg == nullptr ? std::nullopt : std::optional{arg->name};
      }
      if (const pt::Constant* constant = constantAt(tree, target.path);
          constant && isEditableLiteral(constant->value)) {
        return renderLiteral(constant->value);
      }
      return std::nullopt;
    }

    // The edit `text` commits: nullopt rejects the draft, a null edit means the value is unchanged.
    std::optional<std::unique_ptr<Transaction>> commit(const pt::ParseTree& tree,
                                                       const TextEditTool::Target& target,
                                                       const std::string& text) {
      const FullID& path = target.path;
      const std::optional<int> index = indexOf(target);
      if (const pt::Comment* comment = commentAt(tree, path)) {
        return comment->text == text ? nullptr : editComment(path, text);
      }
      if (functionAt(tree, path) != nullptr || callAt(tree, path) != nullptr) {
        if (!isValidIdentifier(text)) {
          return std::nullopt;
        }
        if (text == *draftText(tree, target)) {
          return nullptr;
        }
        if (functionAt(tree, path) != nullptr) {
          return index ? renameParameter(path, *index, text) : renameFunction(path, text);
        }
        return index ? renameCallArgument(path, *index, text) : retargetCall(path, text);
      }
      const pt::Literal& value = constantAt(tree, path)->value;
      const std::optional<pt::Literal> parsed = tryParseLiteral(value, text);
      if (!parsed) {
        return std::nullopt;
      }
      return *parsed == value ? nullptr : setConstantValue(path, *parsed);
    }

    // A function's name or parameter sits on header chrome; everything else on its node.
    Color coverColor(const pt::ParseTree& tree, const FullID& path, const EditorContext::Theme& theme) {
      if (const pt::FunctionDecl* fn = functionAt(tree, path)) {
        return draw::color(*fn, theme);
      }
      if (commentAt(tree, path) != nullptr) {
        return theme.commentNode;
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
    if (field_ && !draftText(state.editor.tree(), target_)) {
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
    const pt::ParseTree& tree = state.editor.tree();
    const Vec2 world = state.view.screenToWorld(event.pos);
    // A terminal press starts a conduit, so it only closes the draft.
    if (terminalAt(tree, boxes, world, state.ctx.layout)) {
      field_.reset();
      return;
    }
    const std::optional<Target> target = targetAt(tree, boxes, world, state.ctx.layout);
    const std::optional<Label> label =
      target ? labelRect(tree, boxes, *target, state.ctx.layout) : std::optional<Label>{};
    if (!label || !label->rect.contains(world)) {
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
    const std::string text = *draftText(tree, *target);
    target_ = *target;
    field_.emplace(text, caretAt(text));
  }

  bool TextEditTool::onKey(InputEvent::Key key, EditorState& state) {
    switch (key) {
      case InputEvent::Key::Return:
        {
          std::optional<std::unique_ptr<Transaction>> edit = commit(state.editor.tree(), target_, field_->text());
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
    const std::optional<Label> label = labelRect(tree, boxes, target_, state.ctx.layout);
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
