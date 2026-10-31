#include "editor/commands/delete_selection.hpp"

#include <algorithm>

#include "editor/diagram/node.hpp"

namespace fluir::editor {

  DeleteSelection::DeleteSelection(const QList<QGraphicsItem*>& selected) : scene_(selected.front()->scene()) {
    for (QGraphicsItem* item : selected) {
      auto* node = dynamic_cast<diagram::FluirNode*>(item);
      if (node != nullptr && node->parentItem() != nullptr) {
        for (QGraphicsItem* sibling : node->parentItem()->childItems()) {
          auto* conduit = dynamic_cast<diagram::Conduit*>(sibling);
          if (conduit == nullptr) {
            continue;
          }
          std::vector<diagram::Terminal*> sinks;
          std::ranges::copy_if(
            conduit->sinks(), std::back_inserter(sinks), [node](auto* sink) { return !node->isAncestorOf(sink); });
          if (node->isAncestorOf(conduit->source()) || sinks.empty()) {
            remove(conduit);
          } else if (sinks.size() != conduit->sinks().size()) {
            rewires_.push_back({.conduit = conduit, .before = conduit->sinks(), .after = std::move(sinks)});
          }
        }
      }
      remove(item);
    }
  }

  DeleteSelection::~DeleteSelection() {
    if (removed_) {
      for (const Removal& removal : removals_) {
        delete removal.item;
      }
    }
  }

  void DeleteSelection::redo() {
    for (const Rewire& rewire : rewires_) {
      rewire.conduit->setSinks(rewire.after);
    }
    for (const Removal& removal : removals_) {
      if (auto* conduit = dynamic_cast<diagram::Conduit*>(removal.item)) {
        conduit->detach();
      }
      scene_->removeItem(removal.item);
    }
    removed_ = true;
  }

  void DeleteSelection::undo() {
    for (auto removal = removals_.rbegin(); removal != removals_.rend(); ++removal) {
      if (removal->parent != nullptr) {
        removal->item->setParentItem(removal->parent);
      } else {
        scene_->addItem(removal->item);
      }
      if (auto* conduit = dynamic_cast<diagram::Conduit*>(removal->item)) {
        conduit->attach();
      }
    }
    for (const Rewire& rewire : rewires_) {
      rewire.conduit->setSinks(rewire.before);
    }
    removed_ = false;
  }

  void DeleteSelection::remove(QGraphicsItem* item) {
    if (std::ranges::none_of(removals_, [item](const Removal& removal) { return removal.item == item; })) {
      removals_.push_back({.item = item, .parent = item->parentItem()});
    }
  }

}  // namespace fluir::editor
