#ifndef FLUIR_EDITOR_CORE_NODE_ACCESS_HPP
#define FLUIR_EDITOR_CORE_NODE_ACCESS_HPP

#include <vector>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/tree.hpp"

namespace fluir::editor {

  fluir::ID idOf(const et::Node& node);
  fluir::ID idOf(const et::Declaration& decl);

  const FlowGraphLocation& locationOf(const et::Node& node);
  const FlowGraphLocation& locationOf(const et::Declaration& decl);

  /** `call`'s arguments, ascending by index. */
  std::vector<const et::Call::Argument*> sortedArguments(const et::Call& call);

  /** `fn`'s parameters, ascending by index; empty without an input block. */
  std::vector<const et::FunctionDecl::Parameter*> sortedParameters(const et::FunctionDecl& fn);

  /** The comment a path names: a top-level declaration or a node in a body, or nullptr. */
  et::Comment* commentAt(et::ParseTree& tree, const FullID& path);
  const et::Comment* commentAt(const et::ParseTree& tree, const FullID& path);

}  // namespace fluir::editor

#endif
