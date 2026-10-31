#include "editor/diagram/function.hpp"

#include <algorithm>
#include <QPainter>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "editor/core/choices.hpp"
#include "editor/core/identifier.hpp"
#include "editor/diagram/parameter.hpp"
#include "editor/diagram/return.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/graph_geometry.hpp"

namespace fluir::editor::diagram {
  namespace {

    // Parameters and the return paint under the body's nodes.
    constexpr double SIGNATURE_Z = -1;

    Rect localRect(const QGraphicsItem& rail) {
      return {0, 0, rail.boundingRect().width(), rail.boundingRect().height()};
    }

    bool hasReturn(const draw::Signature& signature) { return signature.output && signature.output->ret; }

    // The parameter at position `index`, if any.
    template <typename Signature>
    auto* parameterAt(Signature& signature, int index) {
      using Param = std::conditional_t<std::is_const_v<Signature>, const pt::FunctionParameter, pt::FunctionParameter>;
      if (!signature.input) {
        return static_cast<Param*>(nullptr);
      }
      auto& params = signature.input->parameters;
      const auto it = std::ranges::find(params, index, &pt::FunctionParameter::index);
      return it == params.end() ? static_cast<Param*>(nullptr) : &*it;
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
      addFields(draw::labels(signature_, parameter->id(), localRect(*parameter), ctx.layout), *parameter);
    }
    if (return_ != nullptr) {
      addFields(draw::labels(signature_, return_->id(), localRect(*return_), ctx.layout), *return_);
    }
    init(ResizeAxes::XY);
  }

  void Function::arrange() {
    body_->setRect(contentRect(frame(), ctx_.layout.headerH()));
    for (Parameter* parameter : parameters_) {
      parameter->place(draw::railRect(signature_, parameter->id(), frame(), ctx_.layout));
    }
    if (return_ != nullptr) {
      return_->place(draw::railRect(signature_, return_->id(), frame(), ctx_.layout));
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
