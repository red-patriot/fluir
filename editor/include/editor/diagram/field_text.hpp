#ifndef FLUIR_EDITOR_DIAGRAM_FIELD_TEXT_HPP
#define FLUIR_EDITOR_DIAGRAM_FIELD_TEXT_HPP

#include <memory>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsTextItem>
#include <QPainterPath>
#include <QSizeF>

#include "editor/core/editor_context.hpp"
#include "editor/diagram/edit_target.hpp"
#include "editor/diagram/field_part.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::diagram {

  /** A typed field. Unfocused it shows the node's display text; a press opens it on the editable text, all selected,
   *  and selects its node; Enter commits a valid draft as one command, Escape drops it,
   *  and leaving commits it if valid. Shift+Enter breaks a line in comment text and commits elsewhere. */
  class FieldText : public QGraphicsTextItem, public FieldPart {
   public:
    FieldText(FluirNode& node, FieldKey key, FieldLook look, const EditorContext& ctx, QGraphicsItem& parent);

    /** Whether the draft would commit. */
    bool isValid() const { return valid_; }

    void refresh() override;
    void place(const Rect& rect) override;

    /** The placed rect: text past it is clipped and not hittable. */
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override { event->ignore(); }

   private:
    bool multiline() const { return field().kind == Field::Kind::Text; }
    /** Pushes the draft if it is valid and changed; shows the node's value again otherwise. */
    void commit();
    void selectAll();

    const EditorContext& ctx_;
    std::unique_ptr<EditTarget> target_;
    bool valid_ = true;
    bool justFocused_ = false;  // a press that gave focus must not drop the select-all
    QSizeF size_;
  };

}  // namespace fluir::editor::diagram

#endif
