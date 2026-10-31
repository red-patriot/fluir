#include "editor/diagram/node.hpp"

#include <QGraphicsScene>

#include "editor/core/field.hpp"
#include "editor/diagram/choice.hpp"
#include "editor/diagram/field_text.hpp"
#include "editor/diagram/move_grip.hpp"
#include "editor/diagram/resize_grip.hpp"
#include "editor/diagram/toggle.hpp"

namespace fluir::editor::diagram {

  FluirNode::FluirNode(const EditorContext& ctx, QGraphicsItem* parent) : QGraphicsItem(parent), ctx_(ctx) {
    setFlag(ItemIsSelectable);
  }

  void FluirNode::setLocation(const FlowGraphLocation& location) {
    prepareGeometryChange();
    storeLocation(location);
    place();
    arrange();
  }

  void FluirNode::selectAlone() {
    scene()->clearSelection();
    setSelected(true);
  }

  std::string FluirNode::text(Field) const { return {}; }

  bool FluirNode::accepts(Field, std::string_view) const { return false; }

  bool FluirNode::setText(Field field, std::string_view text) {
    if (!accepts(field, text)) {
      return false;
    }
    write(field, text);
    for (FieldPart* part : fields_) {
      if (part->field() == field) {
        part->refresh();
      }
    }
    update();
    return true;
  }

  std::vector<std::string> FluirNode::choices(Field) const { return {}; }

  void FluirNode::write(Field, std::string_view) { }

  QRectF FluirNode::boundingRect() const {
    const Rect local = frame();
    return {local.x, local.y, local.w, local.h};
  }

  void FluirNode::init(std::optional<ResizeAxes> axes) {
    moveGrip_ = new MoveGrip(ctx_, *this);
    if (axes) {
      resizeGrip_ = new ResizeGrip(*axes, ctx_, *this);
    }
    place();
  }

  Rect FluirNode::frame() const {
    const double unit = ctx_.layout.unitPx;
    return {0, 0, location().width * unit, location().height * unit};
  }

  void FluirNode::addFields(const std::vector<FieldLabel>& labels, QGraphicsItem& parent) {
    for (const FieldLabel& label : labels) {
      if (label.field.kind == Field::Kind::Bool) {
        fields_.push_back(new Toggle(*this, label.field, ctx_, parent));
      } else if (isChoice(label.field.kind)) {
        fields_.push_back(new Choice(*this, label.field, ctx_, parent));
      } else {
        fields_.push_back(new FieldText(*this, label.field, ctx_, parent));
      }
      fields_.back()->place(label.rect);
    }
  }

  void FluirNode::placeFields(const std::vector<FieldLabel>& labels) {
    for (const FieldLabel& label : labels) {
      for (FieldPart* part : fields_) {
        if (part->field() == label.field) {
          part->place(label.rect);
        }
      }
    }
  }

  void FluirNode::place() {
    const double unit = ctx_.layout.unitPx;
    setPos(location().x * unit, location().y * unit);
    setZValue(location().z);
    moveGrip_->place(frame());
    if (resizeGrip_ != nullptr) {
      resizeGrip_->place(frame());
    }
  }

}  // namespace fluir::editor::diagram
