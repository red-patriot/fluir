#ifndef FLUIR_EDITOR_SCENE_GRAPH_VIEW_HPP
#define FLUIR_EDITOR_SCENE_GRAPH_VIEW_HPP

#include <QGraphicsView>

#include "editor/core/editor_context.hpp"

namespace fluir::editor {

  /** The canvas an open module's scene is shown in. */
  class GraphView : public QGraphicsView {
   public:
    explicit GraphView(const EditorContext& ctx, QWidget* parent = nullptr);
  };

}  // namespace fluir::editor

#endif
