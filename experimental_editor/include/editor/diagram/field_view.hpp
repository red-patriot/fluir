#ifndef FLUIR_EDITOR_DIAGRAM_FIELD_VIEW_HPP
#define FLUIR_EDITOR_DIAGRAM_FIELD_VIEW_HPP

#include <functional>
#include <memory>
#include <QFont>
#include <QGraphicsItem>
#include <QPoint>
#include <QRectF>
#include <QString>

#include "editor/core/editor_context.hpp"
#include "editor/diagram/edit_target.hpp"
#include "editor/diagram/field_part.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::diagram {

  /** What a press on a field view does to its field. */
  using Press = std::function<void(EditTarget& target, QPoint screenPos)>;

  /** Shows a field's value in its look. A press selects its node and runs `press` on the field. */
  class FieldView : public QGraphicsItem, public FieldPart {
   public:
    FieldView(
      FluirNode& node, FieldKey key, FieldLook look, Press press, const EditorContext& ctx, QGraphicsItem& parent);

    /** The value it shows. */
    const QString& text() const { return text_; }
    /** The font it shows its value in. */
    QFont font() const;

    void refresh() override;
    void place(const Rect& rect) override;

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

   private:
    const EditorContext& ctx_;
    std::unique_ptr<EditTarget> target_;
    FieldLook look_;
    Press press_;
    QRectF rect_;
    QString text_;
  };

}  // namespace fluir::editor::diagram

#endif
