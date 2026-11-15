#include "editor/commands/removed_element.hpp"

namespace fluir::editor {

  RemovedElement::RemovedElement(QGraphicsItem& item) :
    item_(item), parent_(item.parentItem()), scene_(*item.scene()) { }

  void RemovedElement::remove() {
    item_.setParentItem(nullptr);
    scene_.removeItem(&item_);
    held_.reset(&item_);
  }

  void RemovedElement::restore() {
    if (parent_ != nullptr) {
      held_.release()->setParentItem(parent_);
    } else {
      scene_.addItem(held_.release());
    }
  }

}  // namespace fluir::editor
