#ifndef FLUIR_EDITOR_COMMANDS_CONDUIT_CUT_HPP
#define FLUIR_EDITOR_COMMANDS_CONDUIT_CUT_HPP

#include <functional>
#include <QGraphicsItem>
#include <vector>

#include "editor/commands/removed_element.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/terminal.hpp"

namespace fluir::editor {

  /** Deletes the conduit segments on removed terminals off the conduits in `scopes`. */
  class ConduitCut {
   public:
    ConduitCut(const std::vector<QGraphicsItem*>& scopes, const std::function<bool(const diagram::Terminal*)>& cut);

    void apply();
    void restore();

   private:
    struct Removal {
      diagram::Conduit* conduit;
      RemovedElement removed;
    };

    struct Rewire {
      diagram::Conduit* conduit;
      std::vector<diagram::Terminal*> before;
      std::vector<diagram::Terminal*> after;
    };

    std::vector<Removal> removals_;
    std::vector<Rewire> rewires_;
  };

}  // namespace fluir::editor

#endif
