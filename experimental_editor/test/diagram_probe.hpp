#ifndef FLUIR_EDITOR_TEST_DIAGRAM_PROBE_HPP
#define FLUIR_EDITOR_TEST_DIAGRAM_PROBE_HPP

#include <cmath>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QPointF>
#include <vector>

#include "editor/diagram/conduit.hpp"
#include "editor/diagram/terminal.hpp"

namespace testutil {

  /** Every item of type `T` in `scene`. */
  template <typename T>
  std::vector<T*> all(const QGraphicsScene& scene) {
    std::vector<T*> out;
    for (QGraphicsItem* item : scene.items()) {
      if (auto* t = dynamic_cast<T*>(item)) {
        out.push_back(t);
      }
    }
    return out;
  }

  inline bool near(double a, double b) { return std::abs(a - b) < 1e-6; }

  inline bool near(QPointF a, QPointF b) { return near(a.x(), b.x()) && near(a.y(), b.y()); }

  inline QPointF centerOf(const fluir::editor::diagram::Terminal* terminal) {
    return terminal->sceneBoundingRect().center();
  }

  /** Whether `path` (in `item`'s coordinates) has a line from `from` to `to` (in scene coordinates). */
  inline bool hasLine(const QGraphicsItem& item, const QPainterPath& path, QPointF from, QPointF to) {
    for (int i = 0; i + 1 < path.elementCount(); ++i) {
      const QPainterPath::Element a = path.elementAt(i);
      const QPainterPath::Element b = path.elementAt(i + 1);
      if (a.isMoveTo() && b.isLineTo() && near(item.mapToScene(QPointF(a)), from) &&
          near(item.mapToScene(QPointF(b)), to)) {
        return true;
      }
    }
    return false;
  }

  /** Whether `conduit` draws a line from its source terminal to each sink terminal. */
  inline bool routed(const fluir::editor::diagram::Conduit& conduit) {
    for (const fluir::editor::diagram::Terminal* sink : conduit.sinks()) {
      if (!hasLine(conduit, conduit.path(), centerOf(conduit.source()), centerOf(sink))) {
        return false;
      }
    }
    return true;
  }

}  // namespace testutil

#endif
