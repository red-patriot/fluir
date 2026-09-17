#pragma once

#include "editor/core/geometry.hpp"

namespace fluir::editor {

  class Renderer;

  /** Pan + zoom state. screen = world * scale + pan. */
  struct Viewport {
    Vec2 pan;
    double scale = 1.0;

    Vec2 worldToScreen(Vec2 world) const;
    Vec2 screenToWorld(Vec2 screen) const;

    /** Multiply `scale` by `factor`, keeping the world point currently under
     *  `screenPivot` fixed under `screenPivot`. */
    void zoomAbout(Vec2 screenPivot, double factor);

    /** Set `pan`+`scale` so `world` fits centred in a `viewportSize`-pixel
     *  viewport: uniform `scale = min(vp.x/world.w, vp.y/world.h)` clamped to
     *  `[minScale, maxScale]`, then centre. Zero-width or zero-height `world`:
     *  `scale = 1`, still centre. */
    void fitRect(Rect world, Vec2 viewportSize, double minScale, double maxScale);
  };

  /** A composable rendering region. Combines a transform with a local origin
   *  offset and always owns a `Renderer` clip region via RAII: `pushClip` to
   *  `bounds` on construction, `popClip` on destruction. Nested origins add;
   *  nested clips intersect (the renderer's clip stack intersects on push). */
  class Subview {
   public:
    /** Root: `viewport`'s transform unchanged, clipped to `screenClip`. */
    Subview(const Viewport& viewport, Rect screenClip, Renderer& renderer);
    /** Nested: compose `parent`'s transform with `frameLocal`'s top-left origin,
     *  expressed in `parent`'s local world-pixel space, clipped to `frameLocal`'s
     *  size. Inherits `parent`'s renderer. */
    Subview(const Subview& parent, Rect frameLocal);
    ~Subview();

    Subview(const Subview&) = delete;
    Subview& operator=(const Subview&) = delete;
    Subview(Subview&&) = delete;
    Subview& operator=(Subview&&) = delete;
    // No move ctor (reference member). `child()` returns by value via C++17
    // guaranteed copy elision, so no move is needed.

    [[nodiscard]] Subview child(Rect frameLocal) const { return Subview{*this, frameLocal}; }

    Vec2 toScreen(Vec2 local) const { return composed_.worldToScreen(local); }
    Rect toScreen(Rect local) const {
      const Vec2 tl = composed_.worldToScreen(local.topLeft());
      return {tl.x, tl.y, local.w * composed_.scale, local.h * composed_.scale};
    }
    const Viewport& composed() const { return composed_; }
    Renderer& renderer() const { return renderer_; }

   private:
    Viewport composed_;
    Renderer& renderer_;
  };

}  // namespace fluir::editor
