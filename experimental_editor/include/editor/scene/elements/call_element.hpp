#ifndef FLUIR_EDITOR_SCENE_ELEMENTS_CALL_ELEMENT_HPP
#define FLUIR_EDITOR_SCENE_ELEMENTS_CALL_ELEMENT_HPP

#include "editor/core/geometry.hpp"
#include "editor/core/tree.hpp"
#include "editor/scene/elements/element.hpp"

namespace fluir::editor {

  class CallElement : public Element {
   public:
    CallElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent);

    void sync(const et::ParseTree& tree, const et::Call& node, const Rect& world);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    Color color_;
  };

}  // namespace fluir::editor

#endif
