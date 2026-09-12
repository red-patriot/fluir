#include "editor/components/text_field.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>

#include "editor/core/renderer.hpp"

namespace fluir::editor {
  namespace {
    constexpr double GLYPH_PX = 8.0;  ///< SdlRenderer's debug font is a fixed cell
  }  // namespace

  bool TextField::begin(double dxScreenPx) {
    const std::optional<std::string> draft = prefill_ ? prefill_() : std::nullopt;
    if (!draft) {
      active_ = false;
      return false;
    }
    text_ = *draft;
    caret_ = indexAt(text_, dxScreenPx);
    active_ = true;
    invalid_ = false;
    return true;
  }

  void TextField::insert(std::string_view utf8) {
    std::string filtered;
    filtered.reserve(utf8.size());
    for (char c : utf8) {
      if (!std::iscntrl(static_cast<unsigned char>(c))) {
        filtered.push_back(c);
      }
    }
    text_.insert(caret_, filtered);
    caret_ += filtered.size();
    invalid_ = false;
  }

  bool TextField::onKey(InputEvent::Key key) {
    switch (key) {
      case InputEvent::Key::Left:
        caret_ = caret_ > 0 ? caret_ - 1 : 0;
        return true;
      case InputEvent::Key::Right:
        caret_ = caret_ < text_.size() ? caret_ + 1 : text_.size();
        return true;
      case InputEvent::Key::Home:
        caret_ = 0;
        return true;
      case InputEvent::Key::End:
        caret_ = text_.size();
        return true;
      case InputEvent::Key::Backspace:
        if (caret_ > 0) {
          text_.erase(caret_ - 1, 1);
          --caret_;
        }
        return true;
      case InputEvent::Key::Delete:
        if (caret_ < text_.size()) {
          text_.erase(caret_, 1);
        }
        return true;
      default:
        return false;
    }
  }

  void TextField::setCaretFromOffset(double dxScreenPx) { caret_ = indexAt(text_, dxScreenPx); }

  bool TextField::commit(const EditorContext& ctx) {
    if (!commit_ || !commit_(ctx, text_)) {
      invalid_ = true;
      return false;
    }
    active_ = false;
    invalid_ = false;
    return true;
  }

  void TextField::cancel() {
    active_ = false;
    invalid_ = false;
  }

  void TextField::draw(const Subview& view, const EditorContext& ctx, Vec2 localTextPos) const {
    const Vec2 origin = view.toScreen(localTextPos);
    view.renderer().drawText(origin, text_, ctx.theme.text);
    view.renderer().fillRect(Rect{origin.x + GLYPH_PX * static_cast<double>(caret_), origin.y, 1.0, GLYPH_PX},
                             ctx.theme.text);
  }

  std::size_t TextField::indexAt(const std::string& text, double dxScreenPx) {
    const long long idx = std::llround(dxScreenPx / GLYPH_PX);
    return static_cast<std::size_t>(std::clamp<long long>(idx, 0, static_cast<long long>(text.size())));
  }

}  // namespace fluir::editor
