#include "editor/scene/components/toggle.hpp"

#include <QPainter>
#include <utility>

#include "editor/scene/item_style.hpp"

namespace fluir::editor {

  Toggle::Toggle(FullID path, bool value, const Rect& world, const EditorContext& ctx, QGraphicsItem* parent) :
    QGraphicsItem(parent), ctx_(ctx), path_(std::move(path)), value_(value), rect_(style::place(*this, world)) { }

  void Toggle::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::icon(*painter, value_ ? ":/images/true.svg" : ":/images/false.svg", rect_, ctx_.theme.border);
  }

}  // namespace fluir::editor
