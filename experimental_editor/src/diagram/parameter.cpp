#include "editor/diagram/parameter.hpp"

#include <memory>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

#include "editor/commands/delete_parameter.hpp"
#include "editor/diagram/context_menu.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/terminal.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor::diagram {

  Parameter::Parameter(const draw::Signature& signature,
                       fluir::ID id,
                       const Rect& frame,
                       const EditorContext& ctx,
                       QGraphicsItem* function) :
    QGraphicsItem(function), ctx_(ctx), color_(draw::color(signature, ctx.theme)), id_(id) {
    const Rect placed = draw::railRect(signature, id, frame, ctx.layout);
    place(placed);
    rect_ = QRectF(0, 0, placed.w, placed.h);
    setAcceptedMouseButtons(Qt::NoButton);
    addTerminals(*this, draw::anchors(signature, id, Rect{0, 0, placed.w, placed.h}), ctx);
  }

  void Parameter::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::shell(*painter, rect_, color_, ctx_);
  }

  void Parameter::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    auto& function = static_cast<Function&>(*parentItem());
    function.selectAlone();
    auto menu = std::make_unique<QMenu>();
    addEntry(*menu, "Delete", [this, &function] {
      ModuleScene::of(*this).undoStack().push(new DeleteParameter(function, id_));
    });
    popUp(std::move(menu), event->screenPos());
  }

}  // namespace fluir::editor::diagram
