#ifndef FLUIR_EDITOR_SCENE_COMPONENTS_SCOPE_REGION_HPP
#define FLUIR_EDITOR_SCENE_COMPONENTS_SCOPE_REGION_HPP

#include <QGraphicsItem>
#include <QRectF>

#include "editor/core/geometry.hpp"

namespace fluir::editor {

  class ScopeRegion : public QGraphicsItem {
   public:
    explicit ScopeRegion(QGraphicsItem* parent);

    void place(const Rect& world);

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    QRectF rect_;
  };

}  // namespace fluir::editor

#endif
