#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "editor/components/menu.hpp"
#include "editor/tools/popup.hpp"

namespace fluir::editor {

  /** A menu under an anchor: a left press on a row picks it; Escape or a press outside closes. */
  class MenuPopup : public Popup {
   public:
    using OnPick = std::function<void(std::size_t, EditorState&)>;

    /** `anchor` and `bounds` in screen px. `onPick` must not touch `EditorState::popup`. */
    MenuPopup(
      std::vector<std::string> labels, Rect anchor, Rect bounds, const EditorContext::Layout& layout, OnPick onPick);

    bool onEvent(const InputEvent& event, EditorState& state) override;
    void draw(Renderer& renderer, const EditorContext& ctx) const override;

    const std::vector<std::string>& labels() const { return labels_; }

   private:
    std::vector<std::string> labels_;
    MenuLayout layout_;
    OnPick onPick_;
    std::optional<std::size_t> hovered_;
  };

}  // namespace fluir::editor
