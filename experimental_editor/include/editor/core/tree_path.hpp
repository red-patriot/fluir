#pragma once

#include <string>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/tree.hpp"

namespace fluir::editor {

  /** The top-level declaration a one-segment path names, or nullptr. */
  et::Declaration* declarationAt(et::ParseTree& tree, const FullID& path);
  const et::Declaration* declarationAt(const et::ParseTree& tree, const FullID& path);

  /** The function a one-segment path names, or nullptr. */
  et::FunctionDecl* functionAt(et::ParseTree& tree, const FullID& path);
  const et::FunctionDecl* functionAt(const et::ParseTree& tree, const FullID& path);

  /** Depth parity names the kind: even depth >= 2 is a node, odd depth >= 3 a branch. */
  bool isNodePath(const FullID& path);
  bool isBranchPath(const FullID& path);

  /** The block `branchId` names inside `conditional`, or nullptr. The one place a branch index picks a scope. */
  et::Block* branchBlock(et::Conditional& conditional, fluir::ID branchId);
  const et::Block* branchBlock(const et::Conditional& conditional, fluir::ID branchId);

  /** The branch block an odd path of depth >= 3 names inside its parent conditional, or nullptr. */
  et::Block* branchAt(et::ParseTree& tree, const FullID& path);
  const et::Block* branchAt(const et::ParseTree& tree, const FullID& path);

  /** The block a container path owns, or nullptr. The only depth-aware lookup. */
  et::Block* blockOf(et::ParseTree& tree, const FullID& containerPath);
  const et::Block* blockOf(const et::ParseTree& tree, const FullID& containerPath);

  /** The node a path names: its last segment inside its parent's block. */
  et::Node* nodeAt(et::ParseTree& tree, const FullID& path);
  const et::Node* nodeAt(const et::ParseTree& tree, const FullID& path);

  /** The location of whatever the path names or nullptr. */
  FlowGraphLocation* locationAt(et::ParseTree& tree, const FullID& path);
  const FlowGraphLocation* locationAt(const et::ParseTree& tree, const FullID& path);

  /** The type name of `fn`'s param or return with id `railId`, or nullptr. */
  std::string* railTypeAt(et::FunctionDecl& fn, fluir::ID railId);
  const std::string* railTypeAt(const et::FunctionDecl& fn, fluir::ID railId);

  /** `path` without its last segment; empty for an empty path. */
  FullID parentOf(const FullID& path);

}  // namespace fluir::editor
