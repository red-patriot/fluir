#ifndef FLUIR_EDITOR_SCENE_GRAPH_VIEW_HPP
#define FLUIR_EDITOR_SCENE_GRAPH_VIEW_HPP

#include <optional>
#include <QGraphicsView>
#include <QPoint>

#include "editor/core/editor_context.hpp"

namespace fluir::editor {

  /** The canvas an open module's scene is shown in: wheel zooms about the cursor, middle drag pans, F fits. */
  class GraphView : public QGraphicsView {
   public:
    explicit GraphView(const EditorContext& ctx, QWidget* parent = nullptr);

    /** Fits the scene's items in view, within the zoom range. */
    void fit();

    /** View px per scene px. */
    double zoom() const;

   protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

   private:
    const EditorContext& ctx_;
    std::optional<QPoint> panFrom_;
  };

}  // namespace fluir::editor

#endif
