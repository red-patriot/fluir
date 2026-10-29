#ifndef FLUIR_EDITOR_SCENE_COMPONENTS_TERMINAL_HPP
#define FLUIR_EDITOR_SCENE_COMPONENTS_TERMINAL_HPP

#include <cstddef>
#include <QGraphicsItem>
#include <QRectF>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  class Terminal : public QGraphicsItem {
   public:
    Terminal(FullID path, bool output, std::size_t index, Vec2 anchor, const EditorContext& ctx, QGraphicsItem* parent);

    const FullID& modelPath() const { return path_; }
    bool output() const { return output_; }
    std::size_t index() const { return index_; }

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    const EditorContext& ctx_;
    QRectF rect_;
    FullID path_;
    bool output_;
    std::size_t index_;
  };

}  // namespace fluir::editor

#endif
