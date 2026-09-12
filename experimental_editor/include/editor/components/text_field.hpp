#ifndef FLUIR_EDITOR_COMPONENTS_TEXT_FIELD_HPP
#define FLUIR_EDITOR_COMPONENTS_TEXT_FIELD_HPP

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"

namespace fluir::editor {

  /** Reusable in-place text editor: owns an uncommitted draft and caret, raises
   *  exactly one transaction on commit. Knows nothing about the value it
   *  edits -- both seams are owner-supplied callbacks. */
  class TextField {
   public:
    /** Text to prefill the draft with, or nullopt when there is nothing editable. */
    using Prefill = std::function<std::optional<std::string>()>;
    /** Validates and applies the draft; false rejects and keeps it open. */
    using Commit = std::function<bool(const EditorContext&, const std::string&)>;

    TextField(Prefill prefill, Commit commit) : prefill_(std::move(prefill)), commit_(std::move(commit)) { }

    /** Prefills the draft and places the caret at the nearest gap to
     *  `dxScreenPx`. False when the owner declined (prefill returned nullopt). */
    bool begin(double dxScreenPx);

    bool active() const { return active_; }
    bool invalid() const { return invalid_; }

    const std::string& text() const { return text_; }
    std::size_t caret() const { return caret_; }

    /** Inserts printable characters at the caret and advances it; control
     *  characters are ignored. Clears `invalid()`. */
    void insert(std::string_view utf8);

    /** Handles Left/Right/Home/End/Backspace/Delete. Returns whether it handled `key`. */
    bool onKey(InputEvent::Key key);

    void setCaretFromOffset(double dxScreenPx);

    /** Runs the validator once. True closes the field; false marks it
     *  `invalid()` and keeps the draft. */
    bool commit(const EditorContext& ctx);

    /** Closes the field without running the validator. */
    void cancel();

    void draw(const Subview& view, const EditorContext& ctx, Vec2 localTextPos) const;

    /** Nearest caret byte index to `dxScreenPx` from the text's screen origin. */
    static std::size_t indexAt(const std::string& text, double dxScreenPx);

   private:
    Prefill prefill_;
    Commit commit_;
    std::string text_;
    std::size_t caret_ = 0;
    bool active_ = false;
    bool invalid_ = false;
  };

}  // namespace fluir::editor

#endif
