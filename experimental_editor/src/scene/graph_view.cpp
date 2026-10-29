#include "editor/scene/graph_view.hpp"

#include <QColor>

namespace fluir::editor {

  GraphView::GraphView(const EditorContext& ctx, QWidget* parent) : QGraphicsView(parent) {
    const Color& bg = ctx.theme.background;
    setBackgroundBrush(QColor(bg.r, bg.g, bg.b, bg.a));
  }

}  // namespace fluir::editor
