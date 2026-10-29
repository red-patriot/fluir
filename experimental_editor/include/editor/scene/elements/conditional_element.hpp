#ifndef FLUIR_EDITOR_SCENE_ELEMENTS_CONDITIONAL_ELEMENT_HPP
#define FLUIR_EDITOR_SCENE_ELEMENTS_CONDITIONAL_ELEMENT_HPP

#include "editor/core/geometry.hpp"
#include "editor/core/tree.hpp"
#include "editor/scene/elements/element.hpp"

namespace fluir::editor {

  class ConditionalElement : public Element {
   public:
    ConditionalElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent);

    void sync(const et::ParseTree& tree, const et::Conditional& node, const Rect& world);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
  };

}  // namespace fluir::editor

#endif
