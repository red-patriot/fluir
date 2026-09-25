#pragma once

#include "compiler/models/id.hpp"
#include "editor/core/tree.hpp"

namespace fluir::editor {

  /** Erases `nodeId` from `block` and every reference to it. False if it was not there. */
  bool deleteNode(et::Block& block, fluir::ID nodeId);

  /** Erases every conduit sourced from `id` and strips `id` from targets, dropping emptied conduits. */
  void detachConduits(et::Block& block, fluir::ID id);

  /** Whether `conduit` is sourced from or lands on `nodeId`. */
  bool touches(const et::Conduit& conduit, fluir::ID nodeId);

  /** Whether `node` names `nodeId` as an operand. */
  bool hasOperand(const et::Node& node, fluir::ID nodeId);

}  // namespace fluir::editor
