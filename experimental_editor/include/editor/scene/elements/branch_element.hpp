#ifndef FLUIR_EDITOR_SCENE_ELEMENTS_BRANCH_ELEMENT_HPP
#define FLUIR_EDITOR_SCENE_ELEMENTS_BRANCH_ELEMENT_HPP

#include "editor/core/geometry.hpp"
#include "editor/core/tree.hpp"
#include "editor/scene/elements/element.hpp"

namespace fluir::editor {

  class BranchElement : public Element {
   public:
    BranchElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent);

    void sync(const et::Conditional& node, const Rect& frame, const Rect& content);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
  };

}  // namespace fluir::editor

#endif
