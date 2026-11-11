#include "editor/commands/delete_node.hpp"

#include <vector>

namespace fluir::editor {
  namespace {

    // A top-level node has no conduits around it.
    std::vector<QGraphicsItem*> scopeOf(const diagram::FluirNode& node) {
      return node.parentItem() != nullptr ? std::vector<QGraphicsItem*>{node.parentItem()} :
                                            std::vector<QGraphicsItem*>{};
    }

  }  // namespace

  DeleteNode::DeleteNode(diagram::FluirNode& node) :
    cut_(scopeOf(node), [&node](const diagram::Terminal* terminal) { return node.isAncestorOf(terminal); }),
    removed_(node) { }

  void DeleteNode::redo() {
    cut_.apply();
    removed_.remove();
  }

  void DeleteNode::undo() {
    removed_.restore();
    cut_.restore();
  }

}  // namespace fluir::editor
