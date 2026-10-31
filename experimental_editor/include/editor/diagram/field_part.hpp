#ifndef FLUIR_EDITOR_DIAGRAM_FIELD_PART_HPP
#define FLUIR_EDITOR_DIAGRAM_FIELD_PART_HPP

#include "editor/core/field.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor::diagram {

  class FluirNode;

  /** A part that shows and edits one field of its node. */
  class FieldPart {
   public:
    virtual ~FieldPart() = default;

    FluirNode& node() const { return node_; }
    Field field() const { return field_; }

    /** Shows the node's current value. */
    virtual void refresh() = 0;
    /** Covers `rect`, in its parent's coordinates. */
    virtual void place(const Rect& rect) = 0;

   protected:
    FieldPart(FluirNode& node, Field field) : node_(node), field_(field) { }

   private:
    FluirNode& node_;
    Field field_;
  };

}  // namespace fluir::editor::diagram

#endif
