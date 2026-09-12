#include "editor/transaction/resize.hpp"

#include <algorithm>
#include <utility>

#include "editor/actors/actor.hpp"

namespace fluir::editor {

  int ResizeTransaction::clampSize(int size) { return std::clamp(size, MIN_SIZE, MAX_SIZE); }

  bool ResizeTransaction::execute(GraphScene& scene) {
    Actor* actor = scene.find(id_);
    if (actor == nullptr) {
      return false;
    }
    FlowGraphLocation* location = actor->location();
    if (location == nullptr || (location->width == width_ && location->height == height_)) {
      return false;
    }
    std::swap(location->width, width_);
    std::swap(location->height, height_);
    return true;
  }

}  // namespace fluir::editor
