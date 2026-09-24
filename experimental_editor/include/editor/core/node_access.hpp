#ifndef FLUIR_EDITOR_CORE_NODE_ACCESS_HPP
#define FLUIR_EDITOR_CORE_NODE_ACCESS_HPP

#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"

namespace fluir::editor {

  fluir::ID idOf(const pt::Node& node);
  fluir::ID idOf(const pt::Declaration& decl);

  const FlowGraphLocation& locationOf(const pt::Node& node);
  const FlowGraphLocation& locationOf(const pt::Declaration& decl);

  /** `call`'s arguments, ascending by index. */
  std::vector<const pt::Call::Argument*> sortedArguments(const pt::Call& call);

  /** `fn`'s parameters, ascending by index; empty without an input block. */
  std::vector<const pt::FunctionDecl::Parameter*> sortedParameters(const pt::FunctionDecl& fn);

  /** The comment a path names: a top-level declaration or a node in a body, or nullptr. */
  pt::Comment* commentAt(pt::ParseTree& tree, const FullID& path);
  const pt::Comment* commentAt(const pt::ParseTree& tree, const FullID& path);

}  // namespace fluir::editor

#endif
