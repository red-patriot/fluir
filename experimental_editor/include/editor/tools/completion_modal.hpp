#pragma once

#include <string>
#include <vector>

#include "editor/components/menu.hpp"
#include "editor/core/intelligence.hpp"
#include "editor/tools/popup.hpp"

namespace fluir::editor {

  /** A centered modal listing completions; Escape or a press outside closes. Picking is not wired yet. */
  class CompletionModal : public Popup {
   public:
    /** `bounds` in screen px; the modal takes the middle half of it. */
    CompletionModal(std::vector<Completion> completions, Rect bounds, const EditorContext::Layout& layout);

    bool onEvent(const InputEvent& event, EditorState& state) override;
    void draw(Renderer& renderer, const EditorContext& ctx) const override;

    const std::vector<std::string>& labels() const { return labels_; }
    Rect frame() const { return layout_.frame; }

   private:
    std::vector<Completion> completions_;
    std::vector<std::string> labels_;
    MenuLayout layout_;
  };

}  // namespace fluir::editor
