#ifndef FLUIR_EDITOR_ASSETS_IMAGES_HPP
#define FLUIR_EDITOR_ASSETS_IMAGES_HPP

#include <span>

namespace fluir::editor::assets {
  /** Embedded SVG for a drag handle */
  std::span<const unsigned char> dragHandle();
  /** Embedded SVG for a TRUE bool */
  std::span<const unsigned char> trueIcon();
  /** Embedded SVG for a FALSE bool */
  std::span<const unsigned char> falseIcon();
}  // namespace fluir::editor::assets

#endif
