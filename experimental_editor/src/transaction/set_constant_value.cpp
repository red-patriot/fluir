#include "editor/transaction/set_constant_value.hpp"

#include <utility>

#include "editor/actors/actor.hpp"
#include "editor/actors/node_actors.hpp"

namespace fluir::editor {

  bool SetConstantValueTransaction::execute(GraphScene& scene) {
    Actor* actor = scene.find(id_);
    if (actor == nullptr) return false;
    auto* node = dynamic_cast<ConstantActor*>(actor);
    pt::Literal* value = node == nullptr ? nullptr : node->literal();
    if (value == nullptr) return false;
    if (value->index() != value_.index() || *value == value_) return false;  // type change / no-op
    std::swap(*value, value_);
    return true;
  }

}  // namespace fluir::editor
