#ifndef FLUIR_EDITOR_GESTURE_INLINE_EDIT_HPP
#define FLUIR_EDITOR_GESTURE_INLINE_EDIT_HPP

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "editor/components/text_field.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** An actor's optional in-place editor. The draft exists only while it is
   *  open, so a null `field()` is the closed state. The owner supplies the two
   *  seams that are its own: what text prefills, and what a valid value is. */
  class InlineEdit {
   public:
    /** Text to prefill the draft with, or nullopt when there is nothing editable. */
    using Prefill = std::function<std::optional<std::string>()>;
    /** Where the owner draws this draft, in its parent space. */
    using TextOrigin = std::function<Vec2(const EditorContext&)>;

    InlineEdit(Prefill prefill, TextField::Commit commit, TextOrigin origin = {}) :
      prefill_(std::move(prefill)), commit_(std::move(commit)), origin_(std::move(origin)) { }

    /** Opens the draft with the caret nearest `dxScreenPx`, or re-aims the caret
     *  when one is already open. False when the owner has nothing editable. */
    bool begin(double dxScreenPx);

    /** The same, for a press at `parentLocalPos`, measured from the owner's text origin. */
    bool begin(const EditorContext& ctx, Vec2 parentLocalPos);

    bool active() const { return field_ != nullptr; }

    TextField* field() { return field_.get(); }
    const TextField* field() const { return field_.get(); }

    /** Closes the draft without committing. */
    void end() { field_.reset(); }

   private:
    Prefill prefill_;
    TextField::Commit commit_;
    TextOrigin origin_;
    std::unique_ptr<TextField> field_;
  };

}  // namespace fluir::editor

#endif
