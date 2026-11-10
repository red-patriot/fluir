#ifndef FLUIR_EDITOR_DIAGRAM_TOGGLE_HPP
#define FLUIR_EDITOR_DIAGRAM_TOGGLE_HPP

#include <memory>
#include <QGraphicsItem>
#include <QRectF>

#include "editor/core/editor_context.hpp"
#include "editor/diagram/edit_target.hpp"
#include "editor/diagram/field_part.hpp"

namespace fluir::editor::diagram {

  /** A bool field. A press selects its node and flips the value as one command. */
  class Toggle : public QGraphicsItem, public FieldPart {
   public:
    Toggle(FluirNode& node, FieldKey key, const EditorContext& ctx, QGraphicsItem& parent);

    bool value() const { return value_; }

    void refresh() override;
    void place(const Rect& rect) override;

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

   private:
    const EditorContext& ctx_;
    std::unique_ptr<EditTarget> target_;
    QRectF rect_;
    bool value_ = false;
  };

}  // namespace fluir::editor::diagram

#endif
