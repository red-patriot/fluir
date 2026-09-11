#include "editor/transaction/move.hpp"

#include <utility>

#include "editor/actors/actor.hpp"

namespace fluir::editor {

  bool MoveTransaction::execute(GraphScene& scene) {
    Actor* actor = scene.find(id_);
    if (actor == nullptr) {
      return false;
    }
    FlowGraphLocation* location = actor->location();
    if (location == nullptr || (location->x == x_ && location->y == y_)) {
      return false;
    }
    std::swap(location->x, x_);
    std::swap(location->y, y_);
    return true;
  }

}  // namespace fluir::editor
