#ifndef FLUIR_EDITOR_DIAGRAM_CONDITIONAL_HPP
#define FLUIR_EDITOR_DIAGRAM_CONDITIONAL_HPP

#include <memory>
#include <vector>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/diagram/node.hpp"
#include "editor/diagram/port.hpp"
#include "editor/diagram/scope.hpp"
#include "editor/view/draw/conditional.hpp"

namespace fluir::editor::diagram {

  /** Display element for a conditional node. */
  class Conditional : public FluirNode {
   public:
    /** Data about a removed port, so it could be restored later */
    struct RemovedPort {
      std::unique_ptr<Port> item;
      pt::BlockPort data;
      draw::PortRef ref;
    };

    Conditional(
      fluir::ID id, FlowGraphLocation location, draw::Ports ports, const EditorContext& ctx, QGraphicsItem* parent);

    fluir::ID id() const override { return id_; }
    const FlowGraphLocation& location() const override { return location_; }
    Limits<Vec2i> sizeLimits() const override { return draw::sizeLimits(ports_); }
    const draw::Ports& ports() const { return ports_; }
    /** Its wall ports, in `draw::portRefs` order. */
    const std::vector<Port*>& wallPorts() const { return wallPorts_; }
    Scope* thenBranch() const { return then_; }
    Scope* elseBranch() const { return else_; }

    /** Shows the branch that is hidden and hides the one that shows. */
    void switchBranch();

    /** Appends a port with `data` on the `output` side; returns its ref. */
    draw::PortRef addPort(bool output, pt::BlockPort data);
    /** Removes the given port. */
    RemovedPort removePort(draw::PortRef ref);
    /** Puts a `removed` port back. */
    void restorePort(RemovedPort removed);

    /** Slides `port` to `y` grid units below the frame's top. */
    void setPortY(draw::PortRef port, int y);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    void storeLocation(const FlowGraphLocation& location) override { location_ = location; }
    void arrange() override;
    Rect portRect(draw::PortRef port) const;
    pt::BlockPorts& side(bool output) { return output ? ports_.outputs : ports_.inputs; }
    /** Shifts the indices of `output`-side ports from `from` on by `by`. */
    void shiftPorts(bool output, std::size_t from, int by);
    /** Sorts the wall ports into `draw::portRefs` order and places them. */
    void settlePorts();

    fluir::ID id_;
    FlowGraphLocation location_;
    draw::Ports ports_;
    std::vector<Port*> wallPorts_;
    Scope* then_;
    Scope* else_;
  };

}  // namespace fluir::editor::diagram

#endif
