#ifndef FLUIR_EDITOR_COMMANDS_REMOVED_ELEMENT_HPP
#define FLUIR_EDITOR_COMMANDS_REMOVED_ELEMENT_HPP

#include <memory>
#include <QGraphicsItem>
#include <QGraphicsScene>

namespace fluir::editor {

  /** Holds an element while a command has it removed from the scene. */
  class RemovedElement {
   public:
    /** `item` must be in a scene. */
    explicit RemovedElement(QGraphicsItem& item);

    /** Removes the item from the scene and owns it. */
    void remove();
    /** Puts the item back under its parent, or at the scene's top level, and gives up owning it. */
    void restore();

   private:
    QGraphicsItem& item_;
    QGraphicsItem* parent_;
    QGraphicsScene& scene_;
    std::unique_ptr<QGraphicsItem> held_;
  };

}  // namespace fluir::editor

#endif
