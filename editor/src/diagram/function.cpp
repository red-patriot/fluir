#include "editor/diagram/function.hpp"

#include <algorithm>
#include <iterator>
#include <QGraphicsScene>
#include <QMenu>
#include <QPainter>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "editor/commands/add_parameter.hpp"
#include "editor/commands/add_return.hpp"
#include "editor/core/choices.hpp"
#include "editor/core/identifier.hpp"
#include "editor/diagram/context_menu.hpp"
#include "editor/diagram/parameter.hpp"
#include "editor/diagram/return.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/scene/module_scene.hpp"
#include "editor/view/graph_geometry.hpp"

namespace fluir::editor::diagram {
  namespace {

    // Parameters and the return paint under the body's nodes.
    constexpr double SIGNATURE_Z = -1;

    Rect localRect(const QGraphicsItem& item) {
      return {0, 0, item.boundingRect().width(), item.boundingRect().height()};
    }

    // Takes `part` out of its function and the scene, for its taker to own.
    void takeOut(QGraphicsItem& part) {
      QGraphicsScene* scene = part.scene();
      part.setParentItem(nullptr);
      if (scene) {
        scene->removeItem(&part);
      }
    }

    bool hasReturn(const draw::Signature& signature) { return signature.output && signature.output->ret; }

    // The parameter at position `index`, if any.
    template <typename Signature>
    auto* parameterAt(Signature& signature, int index) {
      using Param = std::conditional_t<std::is_const_v<Signature>, const pt::FunctionParameter, pt::FunctionParameter>;
      if (!signature.input || index < 0 || index >= static_cast<int>(signature.input->parameters.size())) {
        return static_cast<Param*>(nullptr);
      }
      return &signature.input->parameters[static_cast<std::size_t>(index)];
    }

  }  // namespace

  Function::Function(fluir::ID id, FlowGraphLocation location, draw::Signature signature, const EditorContext& ctx) :
    FluirNode(ctx, nullptr),
    id_(id),
    location_(location),
    signature_(std::move(signature)),
    body_(new Scope(contentRect(frame(), ctx.layout.headerH()), this)) {
    if (signature_.input) {
      for (const pt::FunctionParameter& param : signature_.input->parameters) {
        parameters_.push_back(new Parameter(signature_, param.id, frame(), ctx, this));
        parameters_.back()->setZValue(SIGNATURE_Z);
      }
    }
    if (signature_.output && signature_.output->ret) {
      return_ = new Return(signature_, signature_.output->ret->id, frame(), ctx, this);
      return_->setZValue(SIGNATURE_Z);
    }
    addFields(draw::labels(signature_, frame(), ctx.layout), *this);
    for (Parameter* parameter : parameters_) {
      addParameterFields(*parameter);
    }
    if (return_ != nullptr) {
      addFields(draw::labels(signature_, return_->id(), localRect(*return_), ctx.layout), *return_);
    }
    init(ResizeAxes::XY);
  }

  void Function::addParameter(pt::FunctionParameter data) {
    if (!signature_.input) {
      signature_.input.emplace();
    }
    signature_.input->parameters.push_back(data);
    auto* parameter = new Parameter(signature_, data.id, frame(), ctx_, this);
    parameter->setZValue(SIGNATURE_Z);
    addParameterFields(*parameter);
    parameters_.push_back(parameter);
    arrange();
  }

  Function::RemovedParameter Function::removeParameter(fluir::ID id) {
    std::vector<pt::FunctionParameter>& params = signature_.input->parameters;
    const auto data = std::ranges::find(params, id, &pt::FunctionParameter::id);
    RemovedParameter removed{
      .item = nullptr, .data = *data, .position = static_cast<std::size_t>(std::distance(params.begin(), data))};
    params.erase(data);
    const auto item = std::ranges::find(parameters_, id, &Parameter::id);
    removed.item.reset(*item);
    parameters_.erase(item);
    takeOut(*removed.item);
    arrange();
    return removed;
  }

  void Function::restoreParameter(RemovedParameter removed) {
    if (!signature_.input) {
      signature_.input.emplace();
    }
    std::vector<pt::FunctionParameter>& params = signature_.input->parameters;
    params.insert(std::next(params.begin(), static_cast<std::ptrdiff_t>(removed.position)), removed.data);
    parameters_.push_back(removed.item.get());
    removed.item.release()->setParentItem(this);
    arrange();
  }

  void Function::addParameterFields(Parameter& parameter) {
    const fluir::ID id = parameter.id();
    addFields(draw::labels(signature_, id, localRect(parameter), ctx_.layout), parameter, [this, id](Field labelled) {
      // A removed parameter's position is past the end, so its fields name no parameter.
      return [this, id, kind = labelled.kind] {
        static const std::vector<pt::FunctionParameter> NONE;
        const std::vector<pt::FunctionParameter>& params = signature_.input ? signature_.input->parameters : NONE;
        return Field{kind,
                     static_cast<int>(std::ranges::find(params, id, &pt::FunctionParameter::id) - params.begin())};
      };
    });
  }

  void Function::dropEmptyInput() {
    if (signature_.input && signature_.input->parameters.empty()) {
      signature_.input.reset();
    }
  }

  void Function::addReturn(pt::FunctionReturn data) {
    if (!signature_.output) {
      signature_.output.emplace();
    }
    signature_.output->ret = data;
    return_ = new Return(signature_, data.id, frame(), ctx_, this);
    return_->setZValue(SIGNATURE_Z);
    addFields(draw::labels(signature_, data.id, localRect(*return_), ctx_.layout), *return_);
  }

  Function::RemovedReturn Function::removeReturn() {
    RemovedReturn removed{.item = std::unique_ptr<Return>(return_), .data = *signature_.output->ret};
    signature_.output->ret.reset();
    return_ = nullptr;
    takeOut(*removed.item);
    return removed;
  }

  void Function::restoreReturn(RemovedReturn removed) {
    if (!signature_.output) {
      signature_.output.emplace();
    }
    signature_.output->ret = removed.data;
    return_ = removed.item.get();
    removed.item.release()->setParentItem(this);
  }

  void Function::dropEmptyOutput() {
    if (signature_.output && !signature_.output->ret) {
      signature_.output.reset();
    }
  }

  void Function::arrange() {
    body_->setRect(contentRect(frame(), ctx_.layout.headerH()));
    for (Parameter* parameter : parameters_) {
      parameter->place(draw::paramOrReturnRect(signature_, parameter->id(), frame(), ctx_.layout));
    }
    if (return_ != nullptr) {
      return_->place(draw::paramOrReturnRect(signature_, return_->id(), frame(), ctx_.layout));
    }
    placeFields(draw::labels(signature_, frame(), ctx_.layout));
  }

  std::string Function::text(Field field) const {
    switch (field.kind) {
      case Field::Kind::Name:
        return signature_.name;
      case Field::Kind::ParamName:
      case Field::Kind::ParamType:
        {
          const pt::FunctionParameter* param = parameterAt(signature_, field.index);
          return param == nullptr ? "" : field.kind == Field::Kind::ParamName ? param->name : param->typeName;
        }
      case Field::Kind::ReturnType:
        return hasReturn(signature_) ? signature_.output->ret->typeName : "";
      default:
        return "";
    }
  }

  bool Function::accepts(Field field, std::string_view text) const {
    switch (field.kind) {
      case Field::Kind::Name:
        return isValidIdentifier(text);
      case Field::Kind::ParamName:
        return parameterAt(signature_, field.index) != nullptr && isValidIdentifier(text);
      case Field::Kind::ParamType:
        return parameterAt(signature_, field.index) != nullptr && std::ranges::contains(builtinTypes(), text);
      case Field::Kind::ReturnType:
        return hasReturn(signature_) && std::ranges::contains(builtinTypes(), text);
      default:
        return false;
    }
  }

  std::vector<std::string> Function::choices(Field field) const {
    const bool type = field.kind == Field::Kind::ParamType || field.kind == Field::Kind::ReturnType;
    return type ? builtinTypes() : std::vector<std::string>{};
  }

  void Function::write(Field field, std::string_view text) {
    switch (field.kind) {
      case Field::Kind::Name:
        signature_.name = text;
        break;
      case Field::Kind::ParamName:
        parameterAt(signature_, field.index)->name = text;
        break;
      case Field::Kind::ParamType:
        parameterAt(signature_, field.index)->typeName = text;
        break;
      default:
        signature_.output->ret->typeName = text;
        break;
    }
  }

  bool Function::addMenuEntries(QMenu& menu, QPointF local) {
    if (local.y() >= frame().y + ctx_.layout.headerH()) {
      return false;
    }
    addEntry(menu, "Add parameter", [this] { ModuleScene::of(*this).undoStack().push(new AddParameter(*this)); });
    addEntry(menu, "Add return", [this] {
      ModuleScene::of(*this).undoStack().push(new AddReturn(*this));
    })->setEnabled(!hasReturn(signature_));
    return true;
  }

  void Function::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    const QRectF band(0, 0, boundingRect().width(), ctx_.layout.headerH());
    painter->fillRect(boundingRect(), style::color(ctx_.theme.background));
    painter->fillRect(band, style::color(draw::color(signature_, ctx_.theme)));
    painter->setPen(QPen(style::color(ctx_.theme.border), 0));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect());
    style::tag(*painter, band, draw::FN_TAG, ctx_);
  }

}  // namespace fluir::editor::diagram
