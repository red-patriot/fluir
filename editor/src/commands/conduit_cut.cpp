#include "editor/commands/conduit_cut.hpp"

#include <algorithm>
#include <iterator>

namespace fluir::editor {

  ConduitCut::ConduitCut(const std::vector<QGraphicsItem*>& scopes,
                         const std::function<bool(const diagram::Terminal*)>& cut) {
    for (QGraphicsItem* scope : scopes) {
      for (QGraphicsItem* child : scope->childItems()) {
        auto* conduit = dynamic_cast<diagram::Conduit*>(child);
        if (!conduit) {
          continue;
        }
        std::vector<diagram::Terminal*> sinks;
        std::ranges::copy_if(conduit->sinks(), std::back_inserter(sinks), [&](const auto* sink) { return !cut(sink); });
        if (cut(conduit->source()) || sinks.empty()) {
          removals_.push_back({.conduit = conduit, .removed = RemovedElement(*conduit)});
        } else if (sinks.size() != conduit->sinks().size()) {
          rewires_.push_back({.conduit = conduit, .before = conduit->sinks(), .after = std::move(sinks)});
        }
      }
    }
  }

  void ConduitCut::apply() {
    for (const Rewire& rewire : rewires_) {
      rewire.conduit->setSinks(rewire.after);
    }
    for (Removal& removal : removals_) {
      removal.conduit->detach();
      removal.removed.remove();
    }
  }

  void ConduitCut::restore() {
    for (Removal& removal : removals_) {
      removal.removed.restore();
      removal.conduit->attach();
    }
    for (const Rewire& rewire : rewires_) {
      rewire.conduit->setSinks(rewire.before);
    }
  }

}  // namespace fluir::editor
