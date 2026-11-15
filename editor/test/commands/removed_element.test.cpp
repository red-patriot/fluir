#include "editor/commands/removed_element.hpp"

#include <optional>
#include <QGraphicsRectItem>
#include <QGraphicsScene>

#include <gtest/gtest.h>

using fluir::editor::RemovedElement;

TEST(RemovedElement, RemovingTakesItOutAndRestoringPutsItBackUnderItsParent) {
  QGraphicsScene scene;
  auto* parent = new QGraphicsRectItem();
  scene.addItem(parent);
  auto* child = new QGraphicsRectItem(parent);
  RemovedElement removed(*child);

  removed.remove();
  EXPECT_EQ(child->scene(), nullptr);
  EXPECT_EQ(child->parentItem(), nullptr);
  EXPECT_FALSE(parent->childItems().contains(child));

  removed.restore();
  EXPECT_EQ(child->scene(), &scene);
  EXPECT_EQ(child->parentItem(), parent);
}

TEST(RemovedElement, RestoringATopLevelItemPutsItBackInTheScene) {
  QGraphicsScene scene;
  auto* item = new QGraphicsRectItem();
  scene.addItem(item);
  RemovedElement removed(*item);

  removed.remove();
  EXPECT_FALSE(scene.items().contains(item));

  removed.restore();
  EXPECT_EQ(item->scene(), &scene);
  EXPECT_EQ(item->parentItem(), nullptr);
}
