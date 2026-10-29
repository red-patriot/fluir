#ifndef FLUIR_EDITOR_SCENE_ELEMENTS_FUNCTION_ELEMENT_HPP
#define FLUIR_EDITOR_SCENE_ELEMENTS_FUNCTION_ELEMENT_HPP

#include "editor/core/tree.hpp"
#include "editor/scene/components/scope_region.hpp"
#include "editor/scene/elements/element.hpp"

namespace fluir::editor {

  class FunctionElement : public Element {
   public:
    FunctionElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent);

    void sync(const et::ParseTree& tree, const et::FunctionDecl& fn);

    ScopeRegion* scope() const { return scope_; }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    ScopeRegion* scope_;
  };

}  // namespace fluir::editor

#endif
