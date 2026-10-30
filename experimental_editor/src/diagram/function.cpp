#include "editor/diagram/function.hpp"

#include <QPainter>
#include <utility>

#include "editor/diagram/parameter.hpp"
#include "editor/diagram/return.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/graph_geometry.hpp"

namespace fluir::editor::diagram {
  namespace {

    // Parameters and the return paint under the body's nodes.
    constexpr double SIGNATURE_Z = -1;

  }  // namespace

  Function::Function(fluir::ID id, FlowGraphLocation location, draw::Signature signature, const EditorContext& ctx) :
    FluirNode(ctx, nullptr),
    id_(id),
    location_(location),
    signature_(std::move(signature)),
    body_(new Scope(contentRect(frame(), ctx.layout.headerH()), this)) {
    place();
    if (signature_.input) {
      for (const pt::FunctionParameter& param : signature_.input->parameters) {
        (new Parameter(signature_, param.id, frame(), ctx, this))->setZValue(SIGNATURE_Z);
      }
    }
    if (signature_.output && signature_.output->ret) {
      (new Return(signature_, signature_.output->ret->id, frame(), ctx, this))->setZValue(SIGNATURE_Z);
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
