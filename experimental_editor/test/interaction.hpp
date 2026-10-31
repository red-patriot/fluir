#ifndef FLUIR_EDITOR_TEST_INTERACTION_HPP
#define FLUIR_EDITOR_TEST_INTERACTION_HPP

#include <optional>
#include <QApplication>
#include <QGraphicsItem>
#include <QMenu>
#include <QPoint>
#include <QPointF>
#include <QString>
#include <QTest>
#include <QUndoStack>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "diagram_probe.hpp"
#include "editor/app/module_page.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/field.hpp"
#include "editor/diagram/export.hpp"
#include "editor/diagram/move_grip.hpp"
#include "editor/diagram/node.hpp"
#include "editor/scene/graph_view.hpp"
#include "editor/scene/module_scene.hpp"
#include "fixture_loader.hpp"

namespace testutil {

  inline fluir::pt::ParseTree loadTree(const std::string& relPath) {
    Loaded loaded = loadFixture(relPath);
    EXPECT_TRUE(loaded.result.tree.has_value()) << relPath;
    return std::move(loaded.result.tree).value_or(fluir::pt::ParseTree{});
  }

  /** A shown module page over a fixture at zoom 1, driven by mouse and keys through its view. */
  class Interaction {
   public:
    explicit Interaction(const std::string& fixture) : page_(ctx_, std::nullopt, loadTree(fixture)) {
      page_.resize(800, 600);
      page_.show();
      page_.activateWindow();
      EXPECT_TRUE(QTest::qWaitForWindowActive(&page_));
      view().resetTransform();
    }

    ~Interaction() {
      if (QMenu* menu = popup()) {
        menu->close();
      }
    }

    Interaction(const Interaction&) = delete;
    Interaction& operator=(const Interaction&) = delete;

    const fluir::editor::EditorContext& ctx() const { return ctx_; }
    double unit() const { return ctx_.layout.unitPx; }
    fluir::editor::GraphView& view() { return *page_.findChild<fluir::editor::GraphView*>(); }
    fluir::editor::ModuleScene& scene() { return *page_.findChild<fluir::editor::ModuleScene*>(); }
    QUndoStack& stack() { return page_.undoStack(); }
    fluir::pt::ParseTree tree() { return fluir::editor::diagram::exportTree(scene()); }

    /** The one item of type `T` with `id`. */
    template <typename T>
    T& find(fluir::ID id) {
      std::vector<T*> found;
      for (T* item : all<T>(scene())) {
        if (item->id() == id) {
          found.push_back(item);
        }
      }
      if (found.size() != 1) {
        throw std::logic_error("expected one item with id " + std::to_string(id));
      }
      return *found.front();
    }

    /** `owner`'s one direct child of type `T`. */
    template <typename T>
    static T& part(const QGraphicsItem& owner) {
      std::vector<T*> found;
      for (QGraphicsItem* child : owner.childItems()) {
        if (auto* t = dynamic_cast<T*>(child)) {
          found.push_back(t);
        }
      }
      if (found.size() != 1) {
        throw std::logic_error("expected one part of the requested type");
      }
      return *found.front();
    }

    /** Centers the view on scene point `target` and returns where it lands on the viewport. */
    QPoint aimAt(QPointF target) {
      view().centerOn(target);
      return view().mapFromScene(target);
    }

    /** Aims at `item`'s center, plus `offset` scene px. */
    QPoint aim(const QGraphicsItem& item, QPointF offset = {}) {
      return aimAt(item.sceneBoundingRect().center() + offset);
    }

    /** `node`'s one field part of type `T` for `field`, among all its descendants. */
    template <typename T>
    static T& fieldOf(const fluir::editor::diagram::FluirNode& node, fluir::editor::Field field) {
      const QList<QGraphicsItem*> top = node.childItems();
      std::vector<QGraphicsItem*> pending(top.begin(), top.end());
      std::vector<T*> found;
      while (!pending.empty()) {
        QGraphicsItem* item = pending.back();
        pending.pop_back();
        if (auto* part = dynamic_cast<T*>(item); part != nullptr && &part->node() == &node && part->field() == field) {
          found.push_back(part);
        }
        const QList<QGraphicsItem*> children = item->childItems();
        pending.insert(pending.end(), children.begin(), children.end());
      }
      if (found.size() != 1) {
        throw std::logic_error("expected one field part");
      }
      return *found.front();
    }

    /** Selects `node` with a click on its move grip. */
    void select(const fluir::editor::diagram::FluirNode& node) {
      click(aim(part<fluir::editor::diagram::MoveGrip>(node)));
    }

    /** Opens `field` with a click, then replaces its text by typing `text` (no Enter). */
    void typeInto(const QGraphicsItem& field, const QString& text) {
      click(aim(field));
      QTest::keyClick(&view(), Qt::Key_A, Qt::ControlModifier);
      QTest::keyClicks(&view(), text);
    }

    /** The open popup menu, or null. */
    static QMenu* popup() { return qobject_cast<QMenu*>(QApplication::activePopupWidget()); }

    /** A point on `node`'s body clear of its terminals, grips and fields. */
    QPoint body(const QGraphicsItem& node) {
      const QRectF rect = node.sceneBoundingRect();
      return aim(node, QPointF(-rect.width() / 4, rect.height() / 2 - 2));
    }

    /** `point` moved by `units` grid units plus `extraPx` view px. */
    QPoint by(QPoint point, double xUnits, double yUnits, QPoint extraPx = {}) const {
      return point + QPoint(static_cast<int>(xUnits * unit()), static_cast<int>(yUnits * unit())) + extraPx;
    }

    void press(QPoint at) { QTest::mousePress(view().viewport(), Qt::LeftButton, {}, at); }
    void moveTo(QPoint at) { QTest::mouseMove(view().viewport(), at); }
    void release(QPoint at) { QTest::mouseRelease(view().viewport(), Qt::LeftButton, {}, at); }
    void click(QPoint at) {
      press(at);
      release(at);
    }
    void drag(QPoint from, QPoint to) {
      press(from);
      moveTo(to);
      release(to);
    }
    void key(Qt::Key key, Qt::KeyboardModifiers modifiers = {}) { QTest::keyClick(&view(), key, modifiers); }

   private:
    fluir::editor::EditorContext ctx_;
    fluir::editor::ModulePage page_;
  };

}  // namespace testutil

#endif
