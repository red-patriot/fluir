#include "editor/tools/text_edit_tool.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "editor/core/identifier.hpp"
#include "editor/core/literal_text.hpp"
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
#include "editor/view/node_view.hpp"

// Editable kinds: a constant's literal, a call's target and argument names, a function's name and parameter names,
// and a comment's text (any text, drawn wrapped). A new kind adds a branch to `targetAt`, `labelRect`, `draftText` and
// `commit` below.

namespace fluir::editor {
  namespace {

    // Matches the header tag graph_draw draws.
    constexpr std::string_view kFnTag = "fn";

    const pt::Constant* constantAt(const pt::ParseTree& tree, const FullID& path) {
      const pt::Node* node = nodeAt(tree, path);
      return node == nullptr ? nullptr : std::get_if<pt::Constant>(node);
    }

    const pt::Call* callAt(const pt::ParseTree& tree, const FullID& path) {
      const pt::Node* node = nodeAt(tree, path);
      return node == nullptr ? nullptr : std::get_if<pt::Call>(node);
    }

    // A top-level comment (one-segment path) or one in a body.
    const pt::Comment* commentAt(const pt::ParseTree& tree, const FullID& path) {
      if (path.size() == 1) {
        const pt::Declaration* decl = declarationAt(tree, path);
        return decl == nullptr ? nullptr : std::get_if<pt::Comment>(decl);
      }
      const pt::Node* node = nodeAt(tree, path);
      return node == nullptr ? nullptr : std::get_if<pt::Comment>(node);
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

    std::vector<const pt::Call::Argument*> sortedArguments(const pt::Call& call) {
      std::vector<const pt::Call::Argument*> args;
      for (const auto& arg : call.arguments) {
        args.push_back(&arg);
      }
      std::ranges::sort(args, {}, &pt::Call::Argument::index);
      return args;
    }

    std::size_t argumentRow(const pt::Call& call, int index) {
      const std::vector<const pt::Call::Argument*> args = sortedArguments(call);
      const auto it = std::ranges::find(args, index, &pt::Call::Argument::index);
      return static_cast<std::size_t>(it - args.begin()) + 1;
    }

    const Box* findBox(std::span<const Box> boxes, Part part, const FullID& path) {
      const auto it = std::ranges::find_if(boxes, [&](const Box& b) { return b.part == part && b.path == path; });
      return it == boxes.end() ? nullptr : &*it;
    }

    // What a press at `world` opens, or nullopt.
    std::optional<TextEditTool::Target> targetAt(const pt::ParseTree& tree,
                                                 std::span<const Box> boxes,
                                                 Vec2 world,
                                                 const EditorContext::Layout& layout) {
      const Box* hit = hitAt(boxes, world);
      if (hit == nullptr || hit->part != Part::Body) {
        return std::nullopt;
      }
      if (commentAt(tree, hit->path) != nullptr) {
        return TextEditTool::Target{hit->path, std::nullopt};
      }
      if (const pt::FunctionDecl* fn = functionAt(tree, hit->path)) {
        if (world.y < hit->world.y + layout.headerH()) {
          return TextEditTool::Target{hit->path, std::nullopt};
        }
        for (const Box& rail : boxes) {
          if (rail.part != Part::Rail || parentOf(rail.path) != hit->path || !rail.world.contains(world) ||
              (rail.clip && !rail.clip->contains(world))) {
            continue;
          }
          if (fn->input) {
            for (const auto& param : fn->input->parameters) {
              if (param.id == rail.path.back()) {
                return TextEditTool::Target{hit->path, param.index};
              }
            }
          }
        }
        return std::nullopt;
      }
      if (const pt::Call* call = callAt(tree, hit->path)) {
        const auto row = static_cast<std::size_t>((world.y - hit->world.y) / layout.railStep());
        if (row == 0) {
          return TextEditTool::Target{hit->path, std::nullopt};
        }
        const std::vector<const pt::Call::Argument*> args = sortedArguments(*call);
        if (row > args.size()) {
          return std::nullopt;
        }
        return TextEditTool::Target{hit->path, args[row - 1]->index};
      }
      if (const pt::Constant* constant = constantAt(tree, hit->path); constant && isEditableLiteral(constant->value)) {
        return TextEditTool::Target{hit->path, std::nullopt};
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
                                   double viewScale,
                                   const EditorContext::Layout& layout) {
      if (const pt::FunctionDecl* fn = functionAt(tree, target.path); fn && target.index) {
        const pt::FunctionDecl::Parameter* param = paramAt(*fn, *target.index);
        FullID railPath = target.path;
        railPath.push_back(param == nullptr ? INVALID_ID : param->id);
        const Box* rail = param == nullptr ? nullptr : findBox(boxes, Part::Rail, railPath);
        if (rail == nullptr) {
          return std::nullopt;
        }
        return Label{splitLabel(rail->world, param->typeName, viewScale, layout).text, rail->clip};
      }
      const Box* body = findBox(boxes, Part::Body, target.path);
      if (body == nullptr) {
        return std::nullopt;
      }
      if (commentAt(tree, target.path) != nullptr) {
        return Label{body->world, body->clip};
      }
      const Rect& r = body->world;
      if (functionAt(tree, target.path) != nullptr) {
        return Label{splitLabel({r.x, r.y, r.w, layout.headerH()}, kFnTag, viewScale, layout).text, body->clip};
      }
      if (const pt::Call* call = callAt(tree, target.path)) {
        if (!target.index) {
          return Label{{r.x, r.y, r.w, std::min(r.h, layout.railStep())}, body->clip};
        }
        if (argumentAt(*call, *target.index) == nullptr) {
          return std::nullopt;
        }
        const double top = r.y + static_cast<double>(argumentRow(*call, *target.index)) * layout.railStep();
        return Label{{r.x, top, r.w, layout.railStep()}, body->clip};
      }
      if (const pt::Constant* constant = constantAt(tree, target.path);
          constant && isEditableLiteral(constant->value)) {
        return Label{splitLabel(r, literalTypeName(constant->value), viewScale, layout).text, body->clip};
      }
      return std::nullopt;
    }

    std::optional<std::string> draftText(const pt::ParseTree& tree, const TextEditTool::Target& target) {
      if (const pt::Comment* comment = commentAt(tree, target.path)) {
        return comment->text;
      }
      if (const pt::FunctionDecl* fn = functionAt(tree, target.path)) {
        if (!target.index) {
          return fn->name;
        }
        const pt::FunctionDecl::Parameter* param = paramAt(*fn, *target.index);
        return param == nullptr ? std::nullopt : std::optional{param->name};
      }
      if (const pt::Call* call = callAt(tree, target.path)) {
        if (!target.index) {
          return call->target;
        }
        const pt::Call::Argument* arg = argumentAt(*call, *target.index);
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
      if (const pt::Comment* comment = commentAt(tree, path)) {
        return comment->text == text ? nullptr : std::make_unique<EditCommentTransaction>(path, text);
      }
      if (functionAt(tree, path) != nullptr || callAt(tree, path) != nullptr) {
        if (!isValidIdentifier(text)) {
          return std::nullopt;
        }
        if (text == *draftText(tree, target)) {
          return nullptr;
        }
        if (functionAt(tree, path) != nullptr) {
          return target.index ? UpdateFuncParamTransaction::rename(path, *target.index, text) :
                                std::unique_ptr<Transaction>{std::make_unique<RenameTransaction>(path, text)};
        }
        return target.index ? std::make_unique<EditCallArgumentTransaction>(path, *target.index, text) :
                              std::unique_ptr<Transaction>{std::make_unique<EditCallNodeTransaction>(path, text)};
      }
      const pt::Literal& value = constantAt(tree, path)->value;
      const std::optional<pt::Literal> parsed = tryParseLiteral(value, text);
      if (!parsed) {
        return std::nullopt;
      }
      return *parsed == value ? nullptr : std::make_unique<SetConstantValueTransaction>(path, *parsed);
    }

    // A function's name or parameter sits on header chrome; everything else on its node.
    Color coverColor(const pt::ParseTree& tree, const FullID& path, const EditorContext::Theme& theme) {
      if (functionAt(tree, path) != nullptr) {
        return theme.funcDeclHeader;
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
    // A port press starts a conduit, so it only closes the draft.
    if (portAt(tree, boxes, world, state.ctx.layout)) {
      field_.reset();
      return;
    }
    const std::optional<Target> target = targetAt(tree, boxes, world, state.ctx.layout);
    const std::optional<Label> label =
      target ? labelRect(tree, boxes, *target, state.view.scale, state.ctx.layout) : std::optional<Label>{};
    if (!label || !label->rect.contains(world)) {
      field_.reset();
      return;
    }
    const bool wrapped = commentAt(tree, target->path) != nullptr;
    // Glyphs are fixed screen px, so the caret offset is measured on screen.
    const double dx = (world.x - textOrigin(label->rect, state.ctx.layout).x) * state.view.scale;
    const auto caretAt = [&](const std::string& text) -> std::size_t {
      if (!wrapped) {
        return TextField::indexAt(text, dx);
      }
      if (state.text == nullptr) {
        return text.size();
      }
      const Rect local = commentTextRect(label->rect, state.ctx.layout);
      const Vec2 topLeft = state.view.worldToScreen(local.topLeft());
      const double scale = state.view.scale;
      return state.text->wrappedIndexAt(
        Rect{topLeft.x, topLeft.y, local.w * scale, local.h * scale}, text, scale, event.pos);
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
    const std::optional<Label> label = labelRect(tree, boxes, target_, view.composed().scale, state.ctx.layout);
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
