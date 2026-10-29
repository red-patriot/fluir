#include "editor/scene/elements/element.hpp"

#include <cstddef>
#include <optional>
#include <QGraphicsRectItem>
#include <QtAlgorithms>
#include <string>
#include <utility>
#include <variant>

#include "editor/core/field.hpp"
#include "editor/core/fields.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/scene/components/choice.hpp"
#include "editor/scene/components/move_grip.hpp"
#include "editor/scene/components/resize_grip.hpp"
#include "editor/scene/components/terminal.hpp"
#include "editor/scene/components/text_edit.hpp"
#include "editor/scene/components/toggle.hpp"
#include "editor/scene/item_style.hpp"

namespace fluir::editor {
  namespace {

    // Components stack over child elements, which take small z values from the scene.
    constexpr double COMPONENTS_Z = 1e6;

    QString textOf(const et::ParseTree& tree, const FullID& path, Field field) {
      return QString::fromStdString(fields::read(tree, path, field).value_or(std::string{}));
    }

    bool boolAt(const et::ParseTree& tree, const FullID& path) {
      const auto* constant = std::get_if<et::Constant>(nodeAt(tree, path));
      const auto* value = constant == nullptr ? nullptr : std::get_if<fluir::literals_types::BOOL>(&constant->value);
      return value != nullptr && *value;
    }

  }  // namespace

  Element::Element(FullID path, const EditorContext& ctx, QGraphicsItem* parent) :
    QGraphicsObject(parent), ctx_(ctx), path_(std::move(path)), components_(new QGraphicsRectItem(this)) {
    components_->setFlag(QGraphicsItem::ItemHasNoContents);
    components_->setZValue(COMPONENTS_Z);
  }

  void Element::place(const Rect& world) {
    prepareGeometryChange();
    rect_ = style::place(*this, world);
  }

  void Element::clearComponents() { qDeleteAll(components_->childItems()); }

  void Element::addLabels(const et::ParseTree& tree, const FullID& fieldPath, const std::vector<FieldLabel>& labels) {
    for (const FieldLabel& label : labels) {
      if (label.field.kind == Field::Kind::Bool) {
        addComponent<Toggle>(fieldPath, boolAt(tree, fieldPath), label.rect, ctx_);
      } else if (isChoice(label.field.kind)) {
        addComponent<Choice>(fieldPath, label.field, textOf(tree, fieldPath, label.field), label.rect, ctx_);
      } else {
        addComponent<TextEdit>(fieldPath, label.field, textOf(tree, fieldPath, label.field), label.rect, ctx_);
      }
    }
  }

  void Element::addTerminals(const FullID& endpointPath, const TerminalSet& set) {
    for (const bool output : {false, true}) {
      const std::vector<Vec2>& anchors = output ? set.outputs : set.inputs;
      for (std::size_t i = 0; i < anchors.size(); ++i) {
        addComponent<Terminal>(endpointPath, output, i, anchors[i], ctx_);
      }
    }
  }

  void Element::addGrips(const Rect& world, std::optional<Part> resize, const Rect& moveGrip) {
    if (resize) {
      const double unit = ctx_.layout.unitPx;
      addComponent<ResizeGrip>(
        *resize, *resize == Part::ResizeXY ? resizeCorner(world, unit) : resizeEdge(world, unit), ctx_);
    }
    addComponent<MoveGrip>(moveGrip, ctx_);
  }

}  // namespace fluir::editor
