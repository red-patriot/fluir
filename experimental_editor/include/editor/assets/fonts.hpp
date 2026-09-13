#pragma once

#include <span>

namespace fluir::editor::assets {

  /** DejaVu Sans Mono TTF bytes, embedded at build time (see src/assets/fonts/LICENSE). */
  std::span<const unsigned char> dejaVuSansMono();

}  // namespace fluir::editor::assets
