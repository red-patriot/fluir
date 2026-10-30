#ifndef FLUIR_EDITOR_DIAGRAM_BINARY_HPP
#define FLUIR_EDITOR_DIAGRAM_BINARY_HPP

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/diagram/node.hpp"

namespace fluir::editor::diagram {

  /** Display element for a binary operator. */
  class Binary : public FluirNode {
   public:
    Binary(pt::Binary data, const EditorContext& ctx, QGraphicsItem* parent);

    const pt::Binary& data() const { return data_; }
    fluir::ID id() const override { return data_.id; }
    const FlowGraphLocation& location() const override { return data_.location; }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    pt::Binary data_;
  };

}  // namespace fluir::editor::diagram

#endif
