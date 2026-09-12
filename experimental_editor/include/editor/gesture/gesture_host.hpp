#ifndef FLUIR_EDITOR_GESTURE_GESTURE_HOST_HPP
#define FLUIR_EDITOR_GESTURE_GESTURE_HOST_HPP

#include <memory>
#include <utility>
#include <vector>

#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"
#include "editor/gesture/gesture.hpp"
#include "editor/gesture/grip.hpp"

namespace fluir::editor {

  class Actor;

  /** An actor's gestures. Owns the whole drag policy -- grip precedence,
   *  snapping, preview, commit and cancel -- so an actor supplies only its grip
   *  table and size limits, and the interactions drive this rather than the
   *  actor. A live gesture exists only while it runs, so null is idle. */
  class GestureHost {
   public:
    GestureHost(const Actor& actor, std::vector<Grip> grips, Limits<Vec2i> sizeLimits) :
      actor_(actor), grips_(std::move(grips)), limits_(sizeLimits) { }

    GestureHost(const GestureHost&) = delete;
    GestureHost& operator=(const GestureHost&) = delete;
    GestureHost(GestureHost&&) = delete;
    GestureHost& operator=(GestureHost&&) = delete;
    ~GestureHost() = default;

    /** Whether `parentLocal` lands on a grip. Grips are not actors, so callers
     *  need this to tell a gesture press from a press on the body. */
    bool onGrip(const EditorContext& ctx, Vec2 parentLocal) const;

    /** Starts the first grip `parentLocal` hits. False when it hits none. */
    bool press(const EditorContext& ctx, Vec2 parentLocal);

    void drag(const EditorContext& ctx, Vec2 worldDelta);

    /** Commits the live gesture, if it changed anything, and drops it. */
    void release(const EditorContext& ctx);

    /** Drops the live gesture without committing. */
    void cancel() { gesture_.reset(); }

    bool active() const { return gesture_ != nullptr; }

    /** `loc` with the live gesture applied, clamped to the size limits. The
     *  clamp runs with or without a gesture, so an out-of-range model value
     *  still lays out at a size the actor is allowed to be. */
    FlowGraphLocation preview(const FlowGraphLocation& loc) const;

    void draw(const Subview& view, const EditorContext& ctx) const;

   private:
    /** `grip`'s rect in parent-space pixels. */
    Rect gripBox(const Grip& grip, const EditorContext& ctx) const;

    /** The actor's location with the preview applied, for placing the grips. */
    FlowGraphLocation previewLocation() const;

    const Actor& actor_;
    std::vector<Grip> grips_;
    Limits<Vec2i> limits_;
    std::unique_ptr<Gesture> gesture_;
  };

}  // namespace fluir::editor

#endif
