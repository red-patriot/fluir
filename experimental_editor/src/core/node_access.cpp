#include "editor/core/node_access.hpp"

#include <algorithm>
#include <variant>

#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  fluir::ID idOf(const pt::Node& node) {
    return std::visit([](const auto& n) { return n.id; }, node);
  }

  fluir::ID idOf(const pt::Declaration& decl) {
    return std::visit([](const auto& d) { return d.id; }, decl);
  }

  const FlowGraphLocation& locationOf(const pt::Node& node) {
    return std::visit([](const auto& n) -> const FlowGraphLocation& { return n.location; }, node);
  }

  const FlowGraphLocation& locationOf(const pt::Declaration& decl) {
    return std::visit([](const auto& d) -> const FlowGraphLocation& { return d.location; }, decl);
  }

  std::vector<const pt::Call::Argument*> sortedArguments(const pt::Call& call) {
    std::vector<const pt::Call::Argument*> args;
    for (const auto& arg : call.arguments) {
      args.push_back(&arg);
    }
    std::ranges::sort(args, {}, &pt::Call::Argument::index);
    return args;
  }

  std::vector<const pt::FunctionDecl::Parameter*> sortedParameters(const pt::FunctionDecl& fn) {
    std::vector<const pt::FunctionDecl::Parameter*> params;
    if (fn.input) {
      for (const auto& param : fn.input->parameters) {
        params.push_back(&param);
      }
    }
    std::ranges::sort(params, {}, &pt::FunctionDecl::Parameter::index);
    return params;
  }

  pt::Comment* commentAt(pt::ParseTree& tree, const FullID& path) {
    if (path.size() == 1) {
      pt::Declaration* decl = declarationAt(tree, path);
      return decl == nullptr ? nullptr : std::get_if<pt::Comment>(decl);
    }
    pt::Node* node = nodeAt(tree, path);
    return node == nullptr ? nullptr : std::get_if<pt::Comment>(node);
  }

  const pt::Comment* commentAt(const pt::ParseTree& tree, const FullID& path) {
    return commentAt(const_cast<pt::ParseTree&>(tree), path);
  }

}  // namespace fluir::editor
