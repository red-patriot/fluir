#include "editor/tools/completion_modal.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>
#include <variant>

#include "editor/core/renderer.hpp"
#include "editor/tools/tool.hpp"
#include "editor/transaction/add_comment.hpp"
#include "editor/transaction/add_decl.hpp"
#include "editor/transaction/add_node.hpp"

namespace fluir::editor {
  namespace {

    constexpr double MODAL_WIDTH_FRACTION = 0.5;
    constexpr double TEXT_SCALE = 1.25;
    constexpr double ROW_GAP_PX = 4.0;
    constexpr double ROW_PAD_PX = 6.0;
    constexpr std::size_t MAX_VISIBLE_ROWS = 10;

    // Legacy editor defaults, in world units.
    constexpr int FUNCTION_W = 40;
    constexpr int FUNCTION_H = 30;
    constexpr int COMMENT_W = 10;
    constexpr int COMMENT_H = 10;
    constexpr int OPERATOR_W = 8;
    constexpr int CONSTANT_W = 12;
    constexpr int CALL_W = 14;
    constexpr int BOOL_CONSTANT_W = 8;
    constexpr int NODE_H = 5;

    template <typename... Fs>
    struct Overloaded : Fs... {
      using Fs::operator()...;
    };

    FlowGraphLocation placed(Coordinate where, int w, int h) {
      return FlowGraphLocation{.x = where.x, .y = where.y, .z = where.z + 1, .width = w, .height = h};
    }

    std::pair<int, int> callSize(const CallFunctionOption& call) {
      int height = NODE_H * (call.parameters.size() + 1);
      int width = CALL_W;
      return {width, height};
    }

  }  // namespace

  CompletionModal::CompletionModal(
    std::vector<Completion> completions, Rect bounds, Renderer* text, Coordinate where, FullID body) :
    completions_(std::move(completions)), where_(where), body_(std::move(body)) {
    for (const Completion& completion : completions_) {
      labels_.emplace_back(completion.label);
    }
    // Tallest label at the drawn scale, so outlines track the real font.
    double lineHeightPx = GLYPH_PX;
    if (text != nullptr) {
      for (const std::string& label : labels_) {
        lineHeightPx = std::max(lineHeightPx, text->measureText(label).y);
      }
    }
    const double rowHeightPx = lineHeightPx * TEXT_SCALE + 2 * ROW_PAD_PX;
    const auto n = static_cast<double>(labels_.size());
    const double w = bounds.w * MODAL_WIDTH_FRACTION;
    contentH_ = n * rowHeightPx + (n + 1) * ROW_GAP_PX;
    rowStep_ = rowHeightPx + ROW_GAP_PX;
    const auto visible = static_cast<double>(std::min(labels_.size(), MAX_VISIBLE_ROWS));
    const double h = std::min(visible * rowHeightPx + (visible + 1) * ROW_GAP_PX, bounds.h);
    layout_.frame = Rect{bounds.x + (bounds.w - w) / 2, bounds.y + (bounds.h - h) / 2, w, h};
    // Rows stack from the frame's top, inset by a gap on every side.
    for (std::size_t i = 0; i < labels_.size(); ++i) {
      layout_.items.push_back(Rect{layout_.frame.x + ROW_GAP_PX,
                                   layout_.frame.y + ROW_GAP_PX + static_cast<double>(i) * (rowHeightPx + ROW_GAP_PX),
                                   layout_.frame.w - 2 * ROW_GAP_PX,
                                   rowHeightPx});
    }
  }

  std::optional<std::size_t> CompletionModal::rowAt(Vec2 screen) const {
    if (!layout_.frame.contains(screen)) {
      return std::nullopt;
    }
    return menuItemAt(layout_, screen + Vec2{0, scroll_});
  }

  bool CompletionModal::onEvent(const InputEvent& event, EditorState& state) {
    switch (event.type) {
      case InputEvent::Type::MouseMove:
        hovered_ = rowAt(event.pos);
        return true;
      case InputEvent::Type::Wheel:
        scroll_ = std::clamp(scroll_ - event.wheel.y * rowStep_, 0.0, contentH_ - layout_.frame.h);
        return true;
      case InputEvent::Type::MouseDown:
        {
          const std::optional<std::size_t> row = rowAt(event.pos);
          if (!row || event.button != InputEvent::Button::Left) {
            return layout_.frame.contains(event.pos);
          }
          const fluir::ID id = state.editor.generateID(body_);
          std::unique_ptr<Transaction> edit = std::visit(
            Overloaded{[&](const FunctionDefOption&) -> std::unique_ptr<Transaction> {
                         return std::make_unique<AddDecl>(body_, id, placed(where_, FUNCTION_W, FUNCTION_H));
                       },
                       [&](const CommentOption&) -> std::unique_ptr<Transaction> {
                         return std::make_unique<AddComment>(body_, id, placed(where_, COMMENT_W, COMMENT_H));
                       },
                       [&](const OperatorOption& op) -> std::unique_ptr<Transaction> {
                         return std::make_unique<AddNode>(body_, id, placed(where_, OPERATOR_W, NODE_H), op);
                       },
                       [&](const ConstantOption& constant) -> std::unique_ptr<Transaction> {
                         const int w =
                           std::holds_alternative<literals_types::BOOL>(constant.value) ? BOOL_CONSTANT_W : CONSTANT_W;
                         return std::make_unique<AddNode>(body_, id, placed(where_, w, NODE_H), constant);
                       },
                       [&](const CallFunctionOption& call) -> std::unique_ptr<Transaction> {
                         auto [w, h] = callSize(call);
                         return std::make_unique<AddNode>(body_, id, placed(where_, w, h), call);
                       }},
            completions_[*row].option);
          state.editor.apply(std::move(edit));
          return false;
        }
      case InputEvent::Type::KeyDown:
        return event.key != InputEvent::Key::Escape;
      default:
        return true;
    }
  }

  void CompletionModal::draw(Renderer& renderer, const EditorContext& ctx) const {
    renderer.fillRect(layout_.frame, ctx.theme.headerBackground);
    renderer.pushClip(layout_.frame);
    for (std::size_t i = 0; i < labels_.size(); ++i) {
      Rect row = layout_.items[i];
      row.y -= scroll_;
      if (hovered_ == i) {
        renderer.fillRect(row, ctx.theme.buttonEnabled);
      }
      renderer.drawRect(row, ctx.theme.border);
      renderer.drawText(Vec2{row.x + ROW_PAD_PX, row.y + ROW_PAD_PX}, labels_[i], ctx.theme.text, TEXT_SCALE);
    }
    renderer.popClip();
    renderer.drawRect(layout_.frame, ctx.theme.border);
  }

}  // namespace fluir::editor
