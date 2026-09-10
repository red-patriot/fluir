#pragma once

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"

namespace fluir::editor {

  /** Erases `nodeId` from `fn` and every reference to it. False if it was not there. */
  bool deleteNode(pt::FunctionDecl& fn, fluir::ID nodeId);

  /** Erases function `functionId` and its whole body. False if it was not there. */
  bool deleteFunction(pt::ParseTree& tree, fluir::ID functionId);

}  // namespace fluir::editor
