#pragma once

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"

namespace fluir::editor {

  /** Erases `nodeId` from `block` and every reference to it. False if it was not there. */
  bool deleteNode(pt::Block& block, fluir::ID nodeId);

  /** Erases every conduit sourced from `id` and strips `id` from targets, dropping emptied conduits. */
  void detachConduits(pt::Block& block, fluir::ID id);

  /** Whether `conduit` is sourced from or lands on `nodeId`. */
  bool touches(const pt::Conduit& conduit, fluir::ID nodeId);

  /** Restacks every conditional in `block` on its branches */
  void normalizeConditionalHeights(pt::Block& block);

  /** Whether `node` names `nodeId` as an operand. */
  bool hasOperand(const pt::Node& node, fluir::ID nodeId);

}  // namespace fluir::editor
