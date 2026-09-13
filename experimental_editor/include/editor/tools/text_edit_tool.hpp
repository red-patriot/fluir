#pragma once

#include <optional>

#include "editor/components/text_field.hpp"
#include "editor/tools/tool.hpp"

namespace fluir::editor {

  /** Left-press on a constant, call label or function header opens a draft; Return commits, Escape or a press
   *  elsewhere closes. */
  class TextEditTool : public Tool {
   public:
    bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) override;
    void cancel(EditorState&) override { field_.reset(); }
    void draw(const Subview& view, const EditorState& state, std::span<const Box> boxes) const override;

    /** The open draft, or nullptr. */
    const TextField* field() const { return field_ ? &*field_ : nullptr; }

   private:
    void onPress(const InputEvent& event, EditorState& state, std::span<const Box> boxes);
    bool onKey(InputEvent::Key key, EditorState& state);

    FullID path_;
    std::optional<TextField> field_;
  };

}  // namespace fluir::editor
