#include "editor/transaction/move.hpp"

#include <memory>
#include <utility>

#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  bool MoveTransaction::execute(et::ParseTree& tree) {
    FlowGraphLocation* location = locationAt(tree, path_);
    if (location == nullptr || (location->x == x_ && location->y == y_)) {
      return false;
    }
    std::swap(location->x, x_);
    std::swap(location->y, y_);
    return true;
  }

  std::unique_ptr<Transaction> moveTo(fluir::FullID path, int x, int y) {
    return std::make_unique<MoveTransaction>(std::move(path), x, y);
  }

}  // namespace fluir::editor
