#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>

#include <gtest/gtest.h>

namespace {

  TEST(QtPlatform, RunsOffscreen) { EXPECT_EQ(QGuiApplication::platformName(), "offscreen"); }

  TEST(QtPlatform, RendersASceneWithText) {
    QGraphicsScene scene;
    scene.addText("fluir")->setFont(QFont("DejaVu Sans Mono", 12));
    QGraphicsView view(&scene);
    view.show();
    QImage blank(64, 32, QImage::Format_ARGB32);
    blank.fill(Qt::white);
    QImage image = blank;
    QPainter painter(&image);
    scene.render(&painter);
    painter.end();
    EXPECT_NE(image, blank);
  }

}  // namespace
