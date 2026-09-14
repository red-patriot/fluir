#include "editor/tools/completion_modal.hpp"

#include <optional>
#include <utility>

namespace fluir::editor {
  namespace {

    constexpr double kModalFraction = 0.5;

  }  // namespace

  CompletionModal::CompletionModal(std::vector<Completion> completions,
                                   Rect bounds,
                                   const EditorContext::Layout& layout) :
    completions_(std::move(completions)) {
    for (const Completion& completion : completions_) {
      labels_.emplace_back(completion.label);
    }
    const double w = bounds.w * kModalFraction;
    const double h = bounds.h * kModalFraction;
    const Rect frame{bounds.x + (bounds.w - w) / 2, bounds.y + (bounds.h - h) / 2, w, h};
    // Rows stack down from the frame's top, full width; the panel keeps the whole frame.
    layout_ = layoutMenu(labels_, Rect{frame.x, frame.y, frame.w, 0}, frame, layout);
    layout_.frame = frame;
  }

  bool CompletionModal::onEvent(const InputEvent& event, EditorState&) {
    switch (event.type) {
      case InputEvent::Type::MouseDown:
        return layout_.frame.contains(event.pos);
      case InputEvent::Type::KeyDown:
        return event.key != InputEvent::Key::Escape;
      default:
        return true;
    }
  }

  void CompletionModal::draw(Renderer& renderer, const EditorContext& ctx) const {
    drawMenu(renderer, labels_, layout_, std::nullopt, ctx);
  }

}  // namespace fluir::editor
