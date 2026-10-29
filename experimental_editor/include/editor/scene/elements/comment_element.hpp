#ifndef FLUIR_EDITOR_SCENE_ELEMENTS_COMMENT_ELEMENT_HPP
#define FLUIR_EDITOR_SCENE_ELEMENTS_COMMENT_ELEMENT_HPP

#include <string>

#include "editor/core/geometry.hpp"
#include "editor/core/tree.hpp"
#include "editor/scene/elements/element.hpp"

namespace fluir::editor {

  class CommentElement : public Element {
   public:
    CommentElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent);

    void sync(const et::ParseTree& tree, const et::Comment& node, const Rect& world);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    Color color_;
    std::string tag_;
  };

}  // namespace fluir::editor

#endif
