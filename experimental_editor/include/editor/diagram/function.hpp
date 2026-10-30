#ifndef FLUIR_EDITOR_DIAGRAM_FUNCTION_HPP
#define FLUIR_EDITOR_DIAGRAM_FUNCTION_HPP

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/diagram/node.hpp"
#include "editor/diagram/scope.hpp"
#include "editor/view/draw/function.hpp"

namespace fluir::editor::diagram {

  /** Display element for a function declaration. */
  class Function : public FluirNode {
   public:
    Function(fluir::ID id, FlowGraphLocation location, draw::Signature signature, const EditorContext& ctx);

    fluir::ID id() const override { return id_; }
    const FlowGraphLocation& location() const override { return location_; }
    const draw::Signature& signature() const { return signature_; }
    /** Where the body's nodes and conduits live. */
    Scope* body() const { return body_; }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    fluir::ID id_;
    FlowGraphLocation location_;
    draw::Signature signature_;
    Scope* body_;
  };

}  // namespace fluir::editor::diagram

#endif
