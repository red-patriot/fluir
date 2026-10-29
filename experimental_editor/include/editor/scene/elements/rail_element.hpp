#ifndef FLUIR_EDITOR_SCENE_ELEMENTS_RAIL_ELEMENT_HPP
#define FLUIR_EDITOR_SCENE_ELEMENTS_RAIL_ELEMENT_HPP

#include "compiler/models/id.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/tree.hpp"
#include "editor/scene/elements/element.hpp"

namespace fluir::editor {

  class RailElement : public Element {
   public:
    RailElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent);

    void sync(const et::ParseTree& tree, const et::FunctionDecl& fn, fluir::ID railId, const Rect& world);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    Color color_;
  };

}  // namespace fluir::editor

#endif
