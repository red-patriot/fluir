#ifndef FLUIR_EDITOR_DIAGRAM_EDITS_HPP
#define FLUIR_EDITOR_DIAGRAM_EDITS_HPP

#include <QPoint>

#include "editor/diagram/edit_target.hpp"

namespace fluir::editor::diagram {

  /** Pops a menu of `target`'s options at `screenPos`; picking one commits it. `target` must outlive the menu. */
  void pick(EditTarget& target, QPoint screenPos);

  /** Commits the opposite of `target`'s bool value. */
  void flip(EditTarget& target);

}  // namespace fluir::editor::diagram

#endif
