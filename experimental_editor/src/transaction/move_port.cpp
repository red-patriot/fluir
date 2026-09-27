#include "editor/transaction/move_port.hpp"

#include <memory>
#include <utility>
#include <variant>

namespace fluir::editor {

  bool MovePortTransaction::execute(et::ParseTree& tree) {
    auto* conditional = std::get_if<et::Conditional>(nodeAt(tree, path_));
    et::BlockPort* port = conditional == nullptr ? nullptr : portOf(*conditional, port_);
    if (port == nullptr || port->y == y_) {
      return false;
    }
    std::swap(port->y, y_);
    return true;
  }

  std::unique_ptr<Transaction> movePortTo(fluir::FullID path, PortRef port, int y) {
    return std::make_unique<MovePortTransaction>(std::move(path), port, y);
  }

}  // namespace fluir::editor
