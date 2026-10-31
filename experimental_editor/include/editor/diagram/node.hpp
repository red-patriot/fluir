#ifndef FLUIR_EDITOR_DIAGRAM_NODE_HPP
#define FLUIR_EDITOR_DIAGRAM_NODE_HPP

#include <functional>
#include <optional>
#include <QGraphicsItem>
#include <QRectF>
#include <string>
#include <string_view>
#include <vector>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/field.hpp"
#include "editor/core/geometry.hpp"
#include "editor/diagram/field_part.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::diagram {

  class MoveGrip;
  class ResizeGrip;

  /** Display element for a node in a Fluir data-flow diagram. */
  class FluirNode : public QGraphicsItem {
   public:
    virtual fluir::ID id() const = 0;
    virtual const FlowGraphLocation& location() const = 0;
    /** How far this node may be resized, in grid units. */
    virtual Limits<Vec2i> sizeLimits() const = 0;

    /** Makes this node the whole selection. */
    void selectAlone();

    /** Moves and sizes this node, carrying its parts along. */
    void setLocation(const FlowGraphLocation& location);

    /** `field`'s value as text; empty for a field this node lacks. */
    virtual std::string text(Field field) const;
    /** Whether `setText` would take `text` for `field`. */
    virtual bool accepts(Field field, std::string_view text) const;
    /** Sets `field` from `text` and refreshes the parts that show it. False, with nothing changed, if refused. */
    bool setText(Field field, std::string_view text);
    /** The options a choice field offers. */
    virtual std::vector<std::string> choices(Field field) const;

    QRectF boundingRect() const override;

   protected:
    FluirNode(const EditorContext& ctx, QGraphicsItem* parent);

    /** Places this node and adds its grips. Call once from the kind's constructor, after its location is set. */
    void init(std::optional<ResizeAxes> axes);
    /** This node's rect in its own coordinates. */
    Rect frame() const;
    /** Adds a part under `parent` for each of `labels`, in `parent`'s coordinates, keyed by `key` (default: as
     * labelled). */
    void addFields(const std::vector<FieldLabel>& labels,
                   QGraphicsItem& parent,
                   const std::function<FieldKey(Field)>& key = {});
    /** Moves each part named in `labels` onto its rect. */
    void placeFields(const std::vector<FieldLabel>& labels);

    const EditorContext& ctx_;

   private:
    virtual void storeLocation(const FlowGraphLocation& location) = 0;
    /** Lays the kind's own parts out over the current frame. */
    virtual void arrange() = 0;
    /** Stores `text`, which `accepts` took, into `field`. */
    virtual void write(Field field, std::string_view text);

    void place();
    std::vector<FieldPart*> fieldParts() const;

    MoveGrip* moveGrip_ = nullptr;
    ResizeGrip* resizeGrip_ = nullptr;
  };

}  // namespace fluir::editor::diagram

#endif
