#pragma once

#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** Logical-unit -> world-pixel scale. */
  // TODO: Delete this or make it a setting
  inline constexpr double UNIT_PX = 5.0;

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
     *  viewport: uniform `scale = min(vp.x/world.w, vp.y/world.h)`, then centre.
     *  Zero-width or zero-height `world`: leave `scale = 1`, still centre. */
    void fitRect(Rect world, Vec2 viewportSize);
  };

}  // namespace fluir::editor
