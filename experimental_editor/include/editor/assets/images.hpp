#ifndef FLUIR_EDITOR_ASSETS_IMAGES_HPP
#define FLUIR_EDITOR_ASSETS_IMAGES_HPP

#include <span>

namespace fluir::editor {
  /** A non-owning view to an SVG image. */
  using SvgView = std::span<const unsigned char>;

  namespace assets {
    /** Embedded SVG for a drag handle */
    SvgView dragHandle();
    /** Embedded SVG for a TRUE bool */
    SvgView trueIcon();
    /** Embedded SVG for a FALSE bool */
    SvgView falseIcon();
    /** Embedded SVG for XY Drag Handle */
    SvgView xyResizeIcon();
  }  // namespace assets
}  // namespace fluir::editor

#endif
