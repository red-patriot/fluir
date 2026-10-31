#include "editor/diagram/choice.hpp"

#include <QAction>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>
#include <string>
#include <utility>

#include "editor/commands/set_field.hpp"
#include "editor/diagram/node.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor::diagram {

  Choice::Choice(FluirNode& node, FieldKey key, const EditorContext& ctx, QGraphicsItem& parent) :
    QGraphicsItem(&parent), FieldPart(node, std::move(key)), ctx_(ctx) {
    refresh();
  }

  void Choice::refresh() {
    text_ = QString::fromStdString(node().text(field()));
    update();
  }

  void Choice::place(const Rect& rect) {
    prepareGeometryChange();
    rect_ = style::rect(rect);
  }

  void Choice::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    const double pad = ctx_.layout.textPad;
    painter->setClipRect(rect_);
    painter->setFont(style::uiFont());
    painter->setPen(style::color(ctx_.theme.text));
    painter->drawText(rect_.adjusted(pad, pad, -pad, -pad), Qt::AlignLeft | Qt::AlignTop, text_);
  }

  void Choice::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
      event->ignore();
      return;
    }
    node().selectAlone();
    auto* menu = new QMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    for (const std::string& option : node().choices(field())) {
      menu->addAction(QString::fromStdString(option));
    }
    QObject::connect(menu, &QMenu::triggered, menu, [this](QAction* action) {
      const std::string picked = action->text().toStdString();
      const std::string current = node().text(field());
      if (picked != current) {
        ModuleScene::of(*this).undoStack().push(new SetField(node(), field(), current, picked));
      }
    });
    menu->popup(event->screenPos());
  }

}  // namespace fluir::editor::diagram
