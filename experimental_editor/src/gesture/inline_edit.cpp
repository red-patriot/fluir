#include "editor/gesture/inline_edit.hpp"

namespace fluir::editor {

  bool InlineEdit::begin(double dxScreenPx) {
    if (field_ != nullptr) {
      field_->setCaretFromOffset(dxScreenPx);
      return true;
    }
    const std::optional<std::string> draft = prefill_ ? prefill_() : std::nullopt;
    if (!draft) {
      return false;
    }
    field_ = std::make_unique<TextField>(*draft, TextField::indexAt(*draft, dxScreenPx), commit_);
    return true;
  }

  bool InlineEdit::begin(const EditorContext& ctx, Vec2 parentLocalPos) {
    return begin(origin_ ? parentLocalPos.x - origin_(ctx).x : parentLocalPos.x);
  }

}  // namespace fluir::editor
