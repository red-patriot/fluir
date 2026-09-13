#include "editor/transaction/move.hpp"

#include <utility>

#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  bool MoveTransaction::execute(pt::ParseTree& tree) {
    FlowGraphLocation* location = locationAt(tree, path_);
    if (location == nullptr || (location->x == x_ && location->y == y_)) {
      return false;
    }
    std::swap(location->x, x_);
    std::swap(location->y, y_);
    return true;
  }

}  // namespace fluir::editor
