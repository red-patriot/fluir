#ifndef FLUIR_EDITOR_DIAGRAM_TERMINAL_HPP
#define FLUIR_EDITOR_DIAGRAM_TERMINAL_HPP

#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QRectF>
#include <QVariant>
#include <vector>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/diagram/scope.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::diagram {

  class Conduit;

  /** Where a conduit attaches. Re-routes its attached conduits whenever it moves in the scene. */
  class Terminal : public QGraphicsItem {
   public:
    /** A terminal centered on `anchor`, in `owner`'s coordinates. */
    Terminal(bool output, int index, Vec2 anchor, const EditorContext& ctx, QGraphicsItem* owner);

    bool isOutput() const { return output_; }
    /** Its index on its side of its endpoint. */
    int index() const { return index_; }
    void setIndex(int index) { index_ = index; }

    /** The scope its conduits live in. */
    Scope* scope() const;
    /** The item it is an end of. */
    const QGraphicsItem* endpoint() const;
    /** Whether a conduit already carries this output to `sink`. */
    bool feeds(const Terminal& sink) const;

    /** Moves the terminal's center to `anchor`, in its owner's coordinates. */
    void setAnchor(Vec2 anchor) { setPos(anchor.x, anchor.y); }

    void attach(Conduit* conduit);
    void detach(Conduit* conduit);

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

   private:
    Terminal* partnerAt(QPointF scenePos) const;
    void endPreview();

    const EditorContext& ctx_;
    std::vector<Conduit*> conduits_;
    // A preview line to the cursor while dragging.
    QGraphicsLineItem* preview_ = nullptr;
    QRectF rect_;
    bool output_;
    int index_;
  };

  /** One endpoint's terminals in index order. */
  struct Terminals {
    std::vector<Terminal*> inputs;
    std::vector<Terminal*> outputs;
  };

  /** Adds a terminal to `owner` at each of `anchors` (in `owner`'s coordinates). */
  Terminals addTerminals(QGraphicsItem& owner, const TerminalSet& anchors, const EditorContext& ctx);

  /** Moves each of `terminals` to its matching anchor in `anchors`. */
  void moveTerminals(const Terminals& terminals, const TerminalSet& anchors);

}  // namespace fluir::editor::diagram

#endif
