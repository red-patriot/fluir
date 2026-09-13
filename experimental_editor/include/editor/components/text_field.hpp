#ifndef FLUIR_EDITOR_COMPONENTS_TEXT_FIELD_HPP
#define FLUIR_EDITOR_COMPONENTS_TEXT_FIELD_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"

namespace fluir::editor {

  /** An open draft and its caret. Constructing one opens the edit and destroying it cancels. */
  class TextField {
   public:
    TextField(std::string text, std::size_t caret) : text_(std::move(text)), caret_(caret) { }

    bool invalid() const { return invalid_; }

    const std::string& text() const { return text_; }
    std::size_t caret() const { return caret_; }

    /** Inserts printable characters at the caret and advances it; control
     *  characters are ignored. Clears `invalid()`. */
    void insert(std::string_view utf8);

    /** Handles Left/Right/Home/End/Backspace/Delete. Returns whether it handled `key`. */
    bool onKey(InputEvent::Key key);

    void setCaretFromOffset(double dxScreenPx);

    /** Marks the draft invalid until the next edit. */
    void reject() { invalid_ = true; }

    void draw(const Subview& view, const EditorContext& ctx, Vec2 localTextPos) const;

    /** Nearest caret byte index to `dxScreenPx` from the text's screen origin. */
    static std::size_t indexAt(const std::string& text, double dxScreenPx);

   private:
    std::string text_;
    std::size_t caret_ = 0;
    bool invalid_ = false;
  };

}  // namespace fluir::editor

#endif
