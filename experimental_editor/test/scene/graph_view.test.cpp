#include "editor/scene/graph_view.hpp"

#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QTest>
#include <QWheelEvent>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"

// The view's contract is its transform: what a wheel, a middle drag and F do to where the scene sits on screen.

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::GraphView;

  struct Harness {
    EditorContext ctx;
    QGraphicsScene scene;
    GraphView view{ctx};

    explicit Harness(QRectF content = {0, 0, 400, 300}) {
      scene.addRect(content);
      view.setScene(&scene);
      view.resize(800, 600);
      view.show();
    }

    QWidget* viewport() { return view.viewport(); }
    QPoint center() { return viewport()->rect().center(); }

    void wheel(QPoint at, int steps) {
      QWheelEvent event(at,
                        view.viewport()->mapToGlobal(at),
                        {},
                        QPoint(0, 120 * steps),
                        Qt::NoButton,
                        Qt::NoModifier,
                        Qt::NoScrollPhase,
                        false);
      QApplication::sendEvent(viewport(), &event);
    }

    void drag(Qt::MouseButton button, QPoint from, QPoint to) {
      QTest::mousePress(viewport(), button, Qt::NoModifier, from);
      QTest::mouseMove(viewport(), to);
      QTest::mouseRelease(viewport(), button, Qt::NoModifier, to);
    }

    // The scene's content rect as it lands on the viewport.
    QRectF onScreen() { return view.mapFromScene(scene.itemsBoundingRect()).boundingRect(); }
  };

}  // namespace

TEST(GraphView, TheWheelZoomsInAndOut) {
  Harness h;
  const double start = h.view.zoom();

  h.wheel(h.center(), 1);
  EXPECT_GT(h.view.zoom(), start);

  h.wheel(h.center(), -2);
  EXPECT_LT(h.view.zoom(), start);
}

TEST(GraphView, TheWheelKeepsTheScenePointUnderTheCursor) {
  Harness h;
  const QPoint cursor{200, 150};
  const QPointF before = h.view.mapToScene(cursor);

  h.wheel(cursor, 1);

  const QPointF after = h.view.mapToScene(cursor);
  EXPECT_NEAR(after.x(), before.x(), 1.0);
  EXPECT_NEAR(after.y(), before.y(), 1.0);
}

TEST(GraphView, TheWheelStopsAtTheZoomRange) {
  Harness h;

  for (int i = 0; i < 100; ++i)
    h.wheel(h.center(), 1);
  EXPECT_DOUBLE_EQ(h.view.zoom(), h.ctx.zoom.max);

  for (int i = 0; i < 100; ++i)
    h.wheel(h.center(), -1);
  EXPECT_DOUBLE_EQ(h.view.zoom(), h.ctx.zoom.min);
}

TEST(GraphView, AMiddleDragCarriesTheSceneWithTheCursor) {
  Harness h;
  const QPoint from = h.center();
  const QPoint to = from + QPoint{40, 30};
  const QPointF grabbed = h.view.mapToScene(from);

  h.drag(Qt::MiddleButton, from, to);

  const QPointF under = h.view.mapToScene(to);
  EXPECT_NEAR(under.x(), grabbed.x(), 1.0);
  EXPECT_NEAR(under.y(), grabbed.y(), 1.0);
}

TEST(GraphView, ALeftDragDoesNotPan) {
  Harness h;
  const QPointF before = h.view.mapToScene(h.center());

  h.drag(Qt::LeftButton, h.center(), h.center() + QPoint{40, 30});

  EXPECT_EQ(h.view.mapToScene(h.center()), before);
}

TEST(GraphView, FFitsTheWholeSceneInView) {
  Harness h;
  h.drag(Qt::MiddleButton, h.center(), h.center() + QPoint{300, 250});
  ASSERT_FALSE(QRectF(h.viewport()->rect()).contains(h.onScreen()));

  QTest::keyClick(&h.view, Qt::Key_F);

  EXPECT_TRUE(QRectF(h.viewport()->rect()).adjusted(-1, -1, 1, 1).contains(h.onScreen()));
}

TEST(GraphView, FitStaysInsideTheZoomRange) {
  Harness tiny{QRectF{0, 0, 5, 5}};
  tiny.view.fit();
  EXPECT_LE(tiny.view.zoom(), tiny.ctx.zoom.max);

  Harness huge{QRectF{0, 0, 100000, 100000}};
  huge.view.fit();
  EXPECT_GE(huge.view.zoom(), huge.ctx.zoom.min);
}

TEST(GraphView, FitCentresASceneTooSmallToFill) {
  Harness tiny{QRectF{0, 0, 5, 5}};

  tiny.view.fit();

  const QPointF centre = tiny.onScreen().center();
  EXPECT_NEAR(centre.x(), tiny.viewport()->rect().center().x(), 1.0);
  EXPECT_NEAR(centre.y(), tiny.viewport()->rect().center().y(), 1.0);
}
