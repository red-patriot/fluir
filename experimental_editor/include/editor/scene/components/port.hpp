#ifndef FLUIR_EDITOR_SCENE_COMPONENTS_PORT_HPP
#define FLUIR_EDITOR_SCENE_COMPONENTS_PORT_HPP

#include <QGraphicsItem>
#include <QRectF>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  class Port : public QGraphicsItem {
   public:
    Port(PortRef port, const Rect& world, const EditorContext& ctx, QGraphicsItem* parent);

    PortRef port() const { return port_; }

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    const EditorContext& ctx_;
    QRectF rect_;
    PortRef port_;
  };

}  // namespace fluir::editor

#endif
