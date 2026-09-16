#include "editor/tools/completion_modal.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
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
    constexpr double CARET_W_PX = 1.0;
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

    std::string lowered(std::string_view text) {
      std::string out;
      out.reserve(text.size());
      for (char c : text) {
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
      }
      return out;
    }

  }  // namespace

  CompletionModal::CompletionModal(
    std::vector<Completion> completions, Rect bounds, Renderer* text, Coordinate where, FullID body) :
    completions_(std::move(completions)), bounds_(bounds), where_(where), body_(std::move(body)) {
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
    rowH_ = lineHeightPx * TEXT_SCALE + 2 * ROW_PAD_PX;
    applyFilter();
  }

  void CompletionModal::relayout() {
    rowStep_ = rowH_ + ROW_GAP_PX;
    const auto shown = static_cast<double>(std::min(visible_.size(), MAX_VISIBLE_ROWS));
    const double w = bounds_.w * MODAL_WIDTH_FRACTION;
    const double h = std::min(2 * ROW_GAP_PX + rowH_ + shown * rowStep_, bounds_.h);
    layout_.frame = Rect{bounds_.x + (bounds_.w - w) / 2, bounds_.y + (bounds_.h - h) / 2, w, h};
    // The search bar is the first row-sized box under the frame's top gap; rows stack below it.
    search_ = Rect{layout_.frame.x + ROW_GAP_PX, layout_.frame.y + ROW_GAP_PX, layout_.frame.w - 2 * ROW_GAP_PX, rowH_};
    const double rowsTop = search_.y + rowH_;
    rows_ = Rect{layout_.frame.x, rowsTop, layout_.frame.w, layout_.frame.y + layout_.frame.h - rowsTop};
    layout_.items.clear();
    for (std::size_t i = 0; i < visible_.size(); ++i) {
      layout_.items.push_back(Rect{layout_.frame.x + ROW_GAP_PX,
                                   rowsTop + ROW_GAP_PX + static_cast<double>(i) * rowStep_,
                                   layout_.frame.w - 2 * ROW_GAP_PX,
                                   rowH_});
    }
    maxScroll_ = std::max(0.0, (static_cast<double>(visible_.size()) - shown) * rowStep_);
  }

  void CompletionModal::applyFilter() {
    const std::string needle = lowered(query_.text());
    visible_.clear();
    for (std::size_t i = 0; i < labels_.size(); ++i) {
      if (needle.empty() || lowered(labels_[i]).find(needle) != std::string::npos) {
        visible_.push_back(i);
      }
    }
    scroll_ = 0;
    hovered_.reset();
    relayout();
  }

  std::optional<std::size_t> CompletionModal::rowAt(Vec2 screen) const {
    if (!layout_.frame.contains(screen) || !rows_.contains(screen)) {
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
        scroll_ = std::clamp(scroll_ - event.wheel.y * rowStep_, 0.0, maxScroll_);
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
            completions_[visible_[*row]].option);
          state.editor.apply(std::move(edit));
          return false;
        }
      case InputEvent::Type::TextInput:
        query_.insert(event.text);
        applyFilter();
        return true;
      case InputEvent::Type::KeyDown:
        if (event.key == InputEvent::Key::Escape) {
          return false;
        }
        // The query owns every other key but Return: unhandled ones still arrive as its TextInput.
        if (event.key && *event.key != InputEvent::Key::Return) {
          query_.onKey(*event.key);
          applyFilter();
        }
        return true;
      default:
        return true;
    }
  }

  void CompletionModal::draw(Renderer& renderer, const EditorContext& ctx) const {
    renderer.fillRect(layout_.frame, ctx.theme.headerBackground);
    renderer.drawRect(search_, ctx.theme.border);
    const std::string& query = query_.text();
    const Vec2 queryOrigin{search_.x + ROW_PAD_PX, search_.y + ROW_PAD_PX};
    if (!query.empty()) {
      renderer.drawText(queryOrigin, query, ctx.theme.text, TEXT_SCALE);
    }
    const double caretX = queryOrigin.x + renderer.measureText(query.substr(0, query_.caret())).x * TEXT_SCALE;
    renderer.fillRect(Rect{caretX, queryOrigin.y, CARET_W_PX, rowH_ - 2 * ROW_PAD_PX}, ctx.theme.text);
    renderer.pushClip(rows_);
    for (std::size_t i = 0; i < visible_.size(); ++i) {
      Rect row = layout_.items[i];
      row.y -= scroll_;
      if (hovered_ == i) {
        renderer.fillRect(row, ctx.theme.buttonEnabled);
      }
      renderer.drawRect(row, ctx.theme.border);
      renderer.drawText(Vec2{row.x + ROW_PAD_PX, row.y + ROW_PAD_PX}, labels_[visible_[i]], ctx.theme.text, TEXT_SCALE);
    }
    renderer.popClip();
    renderer.drawRect(layout_.frame, ctx.theme.border);
  }

}  // namespace fluir::editor
