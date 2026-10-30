#ifndef FLUIR_EDITOR_DIAGRAM_CONDUIT_HPP
#define FLUIR_EDITOR_DIAGRAM_CONDUIT_HPP

#include <QGraphicsPathItem>
#include <vector>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/diagram/terminal.hpp"

namespace fluir::editor::diagram {

  /** Display element for a conduit. */
  class Conduit : public QGraphicsPathItem {
   public:
    Conduit(
      fluir::ID id, Terminal* source, std::vector<Terminal*> sinks, const EditorContext& ctx, QGraphicsItem* scope);

    fluir::ID id() const { return id_; }
    Terminal* source() const { return source_; }
    const std::vector<Terminal*>& sinks() const { return sinks_; }

    /** Redraws a line from the source to each sink, wherever they now are. */
    void route();

   private:
    fluir::ID id_;
    Terminal* source_;
    std::vector<Terminal*> sinks_;
  };

}  // namespace fluir::editor::diagram

#endif
