#ifndef FLUIR_EDITOR_DIAGRAM_CONSTANT_HPP
#define FLUIR_EDITOR_DIAGRAM_CONSTANT_HPP

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/diagram/node.hpp"

namespace fluir::editor::diagram {

  /** Display element for a literal value. */
  class Constant : public FluirNode {
   public:
    Constant(pt::Constant data, const EditorContext& ctx, QGraphicsItem* parent);

    const pt::Constant& data() const { return data_; }
    fluir::ID id() const override { return data_.id; }
    const FlowGraphLocation& location() const override { return data_.location; }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    pt::Constant data_;
  };

}  // namespace fluir::editor::diagram

#endif
