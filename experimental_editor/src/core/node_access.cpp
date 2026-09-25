#include "editor/core/node_access.hpp"

#include <algorithm>
#include <variant>

#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  fluir::ID idOf(const et::Node& node) {
    return std::visit([](const auto& n) { return n.id; }, node);
  }

  fluir::ID idOf(const et::Declaration& decl) {
    return std::visit([](const auto& d) { return d.id; }, decl);
  }

  const FlowGraphLocation& locationOf(const et::Node& node) {
    return std::visit([](const auto& n) -> const FlowGraphLocation& { return n.location; }, node);
  }

  const FlowGraphLocation& locationOf(const et::Declaration& decl) {
    return std::visit([](const auto& d) -> const FlowGraphLocation& { return d.location; }, decl);
  }

  std::vector<const et::Call::Argument*> sortedArguments(const et::Call& call) {
    std::vector<const et::Call::Argument*> args;
    for (const auto& arg : call.arguments) {
      args.push_back(&arg);
    }
    std::ranges::sort(args, {}, &et::Call::Argument::index);
    return args;
  }

  std::vector<const et::FunctionDecl::Parameter*> sortedParameters(const et::FunctionDecl& fn) {
    std::vector<const et::FunctionDecl::Parameter*> params;
    if (fn.input) {
      for (const auto& param : fn.input->parameters) {
        params.push_back(&param);
      }
    }
    std::ranges::sort(params, {}, &et::FunctionDecl::Parameter::index);
    return params;
  }

  et::Comment* commentAt(et::ParseTree& tree, const FullID& path) {
    if (path.size() == 1) {
      et::Declaration* decl = declarationAt(tree, path);
      return decl == nullptr ? nullptr : std::get_if<et::Comment>(decl);
    }
    et::Node* node = nodeAt(tree, path);
    return node == nullptr ? nullptr : std::get_if<et::Comment>(node);
  }

  const et::Comment* commentAt(const et::ParseTree& tree, const FullID& path) {
    return commentAt(const_cast<et::ParseTree&>(tree), path);
  }

}  // namespace fluir::editor
