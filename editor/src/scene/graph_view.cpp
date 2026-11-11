#include "editor/scene/graph_view.hpp"

#include <algorithm>
#include <cmath>
#include <QColor>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>

namespace fluir::editor {
  namespace {

    // Scene px either way from the origin the view may pan over.
    constexpr double PAN_EXTENT = 1e6;
    // One wheel notch, in angle-delta units.
    constexpr double NOTCH = 120.0;

  }  // namespace

  GraphView::GraphView(const EditorContext& ctx, QWidget* parent) : QGraphicsView(parent), ctx_(ctx) {
    const Color& bg = ctx.theme.background;
    setBackgroundBrush(QColor(bg.r, bg.g, bg.b, bg.a));
    // Panning moves the scroll bars, so the view needs room to scroll in every direction.
    setSceneRect(-PAN_EXTENT, -PAN_EXTENT, 2 * PAN_EXTENT, 2 * PAN_EXTENT);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(NoAnchor);
    setResizeAnchor(AnchorViewCenter);
    setRenderHint(QPainter::Antialiasing);
  }

  void GraphView::fit() {
    if (scene() == nullptr || scene()->items().isEmpty()) {
      return;
    }
    const QRectF bounds = scene()->itemsBoundingRect();
    fitInView(bounds, Qt::KeepAspectRatio);
    const double clamped = std::clamp(zoom(), ctx_.zoom.min, ctx_.zoom.max);
    if (clamped != zoom()) {
      setTransform(QTransform::fromScale(clamped, clamped));
    }
    centerOn(bounds.center());
  }

  double GraphView::zoom() const { return transform().m11(); }

  void GraphView::wheelEvent(QWheelEvent* event) {
    const double notches = event->angleDelta().y() / NOTCH;
    const double target = std::clamp(zoom() * std::pow(ctx_.zoom.wheelStep, notches), ctx_.zoom.min, ctx_.zoom.max);
    const QPoint cursor = event->position().toPoint();
    const QPointF anchor = mapToScene(cursor);
    setTransform(QTransform::fromScale(target, target));
    // Pan so the scene point under the cursor stays under it.
    const QPoint drift = mapFromScene(anchor) - cursor;
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + drift.x());
    verticalScrollBar()->setValue(verticalScrollBar()->value() + drift.y());
    event->accept();
  }

  void GraphView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
      panFrom_ = event->position().toPoint();
      event->accept();
      return;
    }
    QGraphicsView::mousePressEvent(event);
  }

  void GraphView::mouseMoveEvent(QMouseEvent* event) {
    if (panFrom_) {
      const QPoint at = event->position().toPoint();
      const QPoint delta = at - *panFrom_;
      horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
      verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
      panFrom_ = at;
      event->accept();
      return;
    }
    QGraphicsView::mouseMoveEvent(event);
  }

  void GraphView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton && panFrom_) {
      panFrom_.reset();
      event->accept();
      return;
    }
    QGraphicsView::mouseReleaseEvent(event);
  }

  void GraphView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_F && event->modifiers() == Qt::NoModifier) {
      fit();
      event->accept();
      return;
    }
    QGraphicsView::keyPressEvent(event);
  }

}  // namespace fluir::editor
