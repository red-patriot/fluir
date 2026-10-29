#ifndef FLUIR_EDITOR_SCENE_COMPONENTS_TOGGLE_HPP
#define FLUIR_EDITOR_SCENE_COMPONENTS_TOGGLE_HPP

#include <QGraphicsItem>
#include <QRectF>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  class Toggle : public QGraphicsItem {
   public:
    Toggle(FullID path, bool value, const Rect& world, const EditorContext& ctx, QGraphicsItem* parent);

    const FullID& modelPath() const { return path_; }
    bool value() const { return value_; }

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    const EditorContext& ctx_;
    FullID path_;
    bool value_;
    QRectF rect_;
  };

}  // namespace fluir::editor

#endif
