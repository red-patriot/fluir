#ifndef FLUIR_EDITOR_COMPONENTS_TEXT_FIELD_HPP
#define FLUIR_EDITOR_COMPONENTS_TEXT_FIELD_HPP

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"

namespace fluir::editor {

  /** An open draft and its caret. Constructing one opens the edit and destroying
   *  it cancels. Validates edits via a callback. */
  class TextField {
   public:
    /** Validates and applies the draft; false rejects and keeps it open. */
    using Commit = std::function<bool(const EditorContext&, const std::string&)>;

    TextField(std::string text, std::size_t caret, Commit commit) :
      text_(std::move(text)), caret_(caret), commit_(std::move(commit)) { }

    bool invalid() const { return invalid_; }

    const std::string& text() const { return text_; }
    std::size_t caret() const { return caret_; }

    /** Inserts printable characters at the caret and advances it; control
     *  characters are ignored. Clears `invalid()`. */
    void insert(std::string_view utf8);

    /** Handles Left/Right/Home/End/Backspace/Delete. Returns whether it handled `key`. */
    bool onKey(InputEvent::Key key);

    void setCaretFromOffset(double dxScreenPx);

    /** Runs the validator once. True lets the owner close the field; false marks
     *  it `invalid()` and keeps the draft. */
    bool commit(const EditorContext& ctx);

    void draw(const Subview& view, const EditorContext& ctx, Vec2 localTextPos) const;

    /** Nearest caret byte index to `dxScreenPx` from the text's screen origin. */
    static std::size_t indexAt(const std::string& text, double dxScreenPx);

   private:
    std::string text_;
    std::size_t caret_ = 0;
    Commit commit_;
    bool invalid_ = false;
  };

}  // namespace fluir::editor

#endif
