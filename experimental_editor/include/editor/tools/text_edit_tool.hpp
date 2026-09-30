#ifndef FLUIR_EDITOR_TOOLS_TEXT_EDIT_TOOL_HPP
#define FLUIR_EDITOR_TOOLS_TEXT_EDIT_TOOL_HPP

#include <optional>

#include "editor/components/text_field.hpp"
#include "editor/core/field.hpp"
#include "editor/tools/tool.hpp"

namespace fluir::editor {

  /** Left-press on a constant, call label or argument row, function header, parameter rail or comment opens a draft;
   *  Return commits, Escape or a press elsewhere closes. */
  class TextEditTool : public Tool {
   public:
    bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) override;
    void cancel(EditorState&) override { field_.reset(); }
    void draw(const Subview& view, const EditorState& state, std::span<const Box> boxes) const override;

    /** The open draft, or nullptr. */
    const TextField* field() const { return field_ ? &*field_ : nullptr; }

    /** What a draft edits: `field` of the node or declaration at `path`. */
    struct Target {
      FullID path;
      Field field;

      friend bool operator==(const Target&, const Target&) = default;
    };

   private:
    void onPress(const InputEvent& event, EditorState& state, std::span<const Box> boxes);
    bool onKey(InputEvent::Key key, EditorState& state);

    Target target_;
    std::optional<TextField> field_;
  };

}  // namespace fluir::editor

#endif
