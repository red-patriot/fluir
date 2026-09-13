#pragma once

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"

namespace fluir::editor {

  /** The function a one-segment path names, or nullptr. */
  pt::FunctionDecl* functionAt(pt::ParseTree& tree, const FullID& path);
  const pt::FunctionDecl* functionAt(const pt::ParseTree& tree, const FullID& path);

  /** The block a container path owns, or nullptr. The only depth-aware lookup. */
  pt::Block* blockOf(pt::ParseTree& tree, const FullID& containerPath);
  const pt::Block* blockOf(const pt::ParseTree& tree, const FullID& containerPath);

  /** The node a path names: its last segment inside its parent's block. */
  pt::Node* nodeAt(pt::ParseTree& tree, const FullID& path);
  const pt::Node* nodeAt(const pt::ParseTree& tree, const FullID& path);

  /** The location of whatever the path names -- function or node -- or nullptr. */
  FlowGraphLocation* locationAt(pt::ParseTree& tree, const FullID& path);
  const FlowGraphLocation* locationAt(const pt::ParseTree& tree, const FullID& path);

  /** `path` without its last segment; empty for an empty path. */
  FullID parentOf(const FullID& path);

}  // namespace fluir::editor
