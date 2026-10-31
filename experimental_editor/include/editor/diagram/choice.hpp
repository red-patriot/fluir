#ifndef FLUIR_EDITOR_DIAGRAM_CHOICE_HPP
#define FLUIR_EDITOR_DIAGRAM_CHOICE_HPP

#include <QGraphicsItem>
#include <QRectF>
#include <QString>

#include "editor/core/editor_context.hpp"
#include "editor/diagram/field_part.hpp"

namespace fluir::editor::diagram {

  /** A field picked from a menu. A press selects its node and pops the node's options; picking a different one pushes
   *  one command. */
  class Choice : public QGraphicsItem, public FieldPart {
   public:
    Choice(FluirNode& node, Field field, const EditorContext& ctx, QGraphicsItem& parent);

    /** The value it shows. */
    const QString& text() const { return text_; }

    void refresh() override;
    void place(const Rect& rect) override;

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

   private:
    const EditorContext& ctx_;
    QRectF rect_;
    QString text_;
  };

}  // namespace fluir::editor::diagram

#endif
