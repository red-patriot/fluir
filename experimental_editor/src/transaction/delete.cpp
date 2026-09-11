#include "editor/transaction/delete.hpp"

#include <utility>
#include <vector>

#include "editor/actors/function_decl_actor.hpp"
#include "editor/actors/node_actor.hpp"
#include "editor/actors/rail_actors.hpp"

namespace fluir::editor {
  namespace {

    // A conduit is wired to `nodeId` when it is sourced from it or lands on it.
    bool touches(const pt::Conduit& conduit, fluir::ID nodeId) {
      if (conduit.input == nodeId) {
        return true;
      }
      for (const pt::Conduit::Output& child : conduit.children) {
        if (child.target == nodeId) {
          return true;
        }
      }
      return false;
    }

  }  // namespace

  bool DeleteTransaction::execute(GraphScene& scene) {
    removed_.clear();
    clearedOperands_.clear();

    // A frame takes its whole body with it; no wires and no operands to mend.
    if (id_.size() == 1) {
      if (dynamic_cast<FunctionDeclActor*>(scene.find(id_)) == nullptr) {
        return false;
      }
      removed_.push_back(scene.detach(id_));
      return true;
    }
    if (id_.size() != 2) {
      return false;
    }

    auto* frame = dynamic_cast<FunctionDeclActor*>(scene.find(id_[0]));
    if (frame == nullptr || dynamic_cast<NodeActor*>(scene.find(id_)) == nullptr) {
      return false;
    }

    // Collect first: detaching rewrites the child list being walked.
    std::vector<fluir::FullID> wires;
    for (const auto& child : frame->body().children()) {
      const auto* conduit = dynamic_cast<const ConduitActor*>(child.get());
      if (conduit != nullptr && touches(conduit->conduit(), id_.back())) {
        wires.push_back(*conduit->selectionId());
      }
      if (auto* node = dynamic_cast<NodeActor*>(child.get())) {
        for (const int slot : node->clearOperands(id_.back())) {
          clearedOperands_.push_back({*node->selectionId(), slot});
        }
      }
    }
    for (const fluir::FullID& wire : wires) {
      removed_.push_back(scene.detach(wire));
    }
    removed_.push_back(scene.detach(id_));
    return true;
  }

  bool DeleteTransaction::unexecute(GraphScene& scene) {
    if (removed_.empty()) {
      return false;
    }
    // Indices were recorded as each detach happened, so undo them last-first.
    while (!removed_.empty()) {
      if (!scene.attach(std::move(removed_.back()))) {
        return false;
      }
      removed_.pop_back();
    }
    for (const ClearedOperand& cleared : clearedOperands_) {
      if (auto* node = dynamic_cast<NodeActor*>(scene.find(cleared.node))) {
        node->restoreOperand(cleared.slot, id_.back());
      }
    }
    clearedOperands_.clear();
    return true;
  }

}  // namespace fluir::editor
