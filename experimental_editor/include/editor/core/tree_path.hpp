#pragma once

#include <string>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"

namespace fluir::editor {

  /** The top-level declaration a one-segment path names, or nullptr. */
  pt::Declaration* declarationAt(pt::ParseTree& tree, const FullID& path);
  const pt::Declaration* declarationAt(const pt::ParseTree& tree, const FullID& path);

  /** The function a one-segment path names, or nullptr. */
  pt::FunctionDecl* functionAt(pt::ParseTree& tree, const FullID& path);
  const pt::FunctionDecl* functionAt(const pt::ParseTree& tree, const FullID& path);

  /** A branch's path segment is its 1-based index in its conditional; the model holds no id. */
  inline constexpr fluir::ID THEN_BRANCH_ID = 1;
  inline constexpr fluir::ID ELSE_BRANCH_ID = 2;

  /** Depth parity names the kind: even depth >= 2 is a node, odd depth >= 3 a branch. */
  bool isNodePath(const FullID& path);
  bool isBranchPath(const FullID& path);

  /** The branch block an odd path of depth >= 3 names inside its parent conditional, or nullptr. */
  pt::Block* branchAt(pt::ParseTree& tree, const FullID& path);
  const pt::Block* branchAt(const pt::ParseTree& tree, const FullID& path);

  /** The block a container path owns, or nullptr. The only depth-aware lookup. */
  pt::Block* blockOf(pt::ParseTree& tree, const FullID& containerPath);
  const pt::Block* blockOf(const pt::ParseTree& tree, const FullID& containerPath);

  /** The node a path names: its last segment inside its parent's block. */
  pt::Node* nodeAt(pt::ParseTree& tree, const FullID& path);
  const pt::Node* nodeAt(const pt::ParseTree& tree, const FullID& path);

  /** The location of whatever the path names or nullptr. */
  FlowGraphLocation* locationAt(pt::ParseTree& tree, const FullID& path);
  const FlowGraphLocation* locationAt(const pt::ParseTree& tree, const FullID& path);

  /** The type name of `fn`'s param or return with id `railId`, or nullptr. */
  std::string* railTypeAt(pt::FunctionDecl& fn, fluir::ID railId);
  const std::string* railTypeAt(const pt::FunctionDecl& fn, fluir::ID railId);

  /** `path` without its last segment; empty for an empty path. */
  FullID parentOf(const FullID& path);

}  // namespace fluir::editor
