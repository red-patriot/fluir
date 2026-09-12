#include "editor/gesture/gesture_host.hpp"

#include <algorithm>
#include <optional>

#include "compiler/models/id.hpp"
#include "editor/actors/actor.hpp"
#include "editor/core/renderer.hpp"

namespace fluir::editor {

  FlowGraphLocation GestureHost::preview(const FlowGraphLocation& loc) const {
    FlowGraphLocation out = gesture_ ? gesture_->preview(loc) : loc;
    out.width = std::clamp(out.width, limits_.lower.x, limits_.upper.x);
    out.height = std::clamp(out.height, limits_.lower.y, limits_.upper.y);
    return out;
  }

  FlowGraphLocation GestureHost::previewLocation() const {
    const FlowGraphLocation* loc = actor_.location();
    return loc == nullptr ? FlowGraphLocation{} : preview(*loc);
  }

  Rect GestureHost::gripBox(const Grip& grip, const EditorContext& ctx) const {
    const Rect frame = grip.frame(actor_, ctx);
    const Rect box = grip.rect(previewLocation());
    const double unit = ctx.layout.unitPx;
    return {frame.x + box.x * unit, frame.y + box.y * unit, box.w * unit, box.h * unit};
  }

  bool GestureHost::onGrip(const EditorContext& ctx, Vec2 parentLocal) const {
    for (const Grip& grip : grips_) {
      if (gripBox(grip, ctx).contains(parentLocal)) {
        return true;
      }
    }
    return false;
  }

  bool GestureHost::press(const EditorContext& ctx, Vec2 parentLocal) {
    for (const Grip& grip : grips_) {
      if (gripBox(grip, ctx).contains(parentLocal)) {
        gesture_ = grip.begin();
        return true;
      }
    }
    return false;
  }

  void GestureHost::drag(const EditorContext& ctx, Vec2 worldDelta) {
    if (gesture_) {
      gesture_->update(ctx, worldDelta);
    }
  }

  void GestureHost::release(const EditorContext& ctx) {
    if (!gesture_) {
      return;
    }
    const FlowGraphLocation* loc = actor_.location();
    const std::optional<fluir::FullID> id = actor_.selectionId();
    if (loc != nullptr && id) {
      gesture_->commit(ctx, *loc, *id);
    }
    gesture_.reset();
  }

  void GestureHost::draw(const Subview& view, const EditorContext& ctx) const {
    for (const Grip& grip : grips_) {
      view.renderer().fillRect(view.toScreen(gripBox(grip, ctx)), ctx.theme.border);
    }
  }

}  // namespace fluir::editor
