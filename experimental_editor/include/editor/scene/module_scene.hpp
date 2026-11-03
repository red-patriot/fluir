#ifndef FLUIR_EDITOR_SCENE_MODULE_SCENE_HPP
#define FLUIR_EDITOR_SCENE_MODULE_SCENE_HPP

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QUndoStack>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/intelligence.hpp"

namespace fluir::editor {

  /** A module's data-flow diagram. Its items push their edits onto `undoStack` and ask `intelligence` what to offer. */
  class ModuleScene : public QGraphicsScene {
   public:
    ModuleScene(const EditorContext& ctx,
                QUndoStack& undoStack,
                const Intelligence& intelligence,
                QObject* parent = nullptr) :
      QGraphicsScene(parent), ctx_(ctx), undoStack_(undoStack), intelligence_(intelligence) { }

    /** The scene `item` is in. */
    static ModuleScene& of(const QGraphicsItem& item) { return static_cast<ModuleScene&>(*item.scene()); }

    const EditorContext& context() const { return ctx_; }
    QUndoStack& undoStack() const { return undoStack_; }
    const Intelligence& intelligence() const { return intelligence_; }

    const pt::Header& header() const { return header_; }
    void setHeader(const pt::Header& header) { header_ = header; }

   protected:
    /** Escape clears the selection and Delete removes it, unless the focused item took the key. */
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

   private:
    const EditorContext& ctx_;
    QUndoStack& undoStack_;
    const Intelligence& intelligence_;
    pt::Header header_;
  };

}  // namespace fluir::editor

#endif
