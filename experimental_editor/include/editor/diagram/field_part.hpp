#ifndef FLUIR_EDITOR_DIAGRAM_FIELD_PART_HPP
#define FLUIR_EDITOR_DIAGRAM_FIELD_PART_HPP

#include <functional>
#include <utility>

#include "editor/core/field.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor::diagram {

  class FluirNode;

  /** Names a part's field now; a parameter's field follows its parameter's position. */
  using FieldKey = std::function<Field()>;

  /** A part that shows and edits one field of its node. */
  class FieldPart {
   public:
    virtual ~FieldPart() = default;

    FluirNode& node() const { return node_; }
    Field field() const { return key_(); }

    /** Shows the node's current value. */
    virtual void refresh() = 0;
    /** Covers `rect`, in its parent's coordinates. */
    virtual void place(const Rect& rect) = 0;

   protected:
    FieldPart(FluirNode& node, FieldKey key) : node_(node), key_(std::move(key)) { }

   private:
    FluirNode& node_;
    FieldKey key_;
  };

}  // namespace fluir::editor::diagram

#endif
