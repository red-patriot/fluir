#include "editor/core/scene_to_tree.hpp"

#include <utility>
#include <variant>

#include "editor/actors/function_decl_actor.hpp"
#include "editor/actors/node_actor.hpp"
#include "editor/actors/rail_actors.hpp"

namespace fluir::editor {
  namespace {

    // The frame's body children are the function's whole content: rails, nodes and conduits.
    pt::FunctionDecl functionOf(const FunctionDeclActor& frame) {
      pt::FunctionDecl fn;
      fn.id = frame.functionId();
      fn.location = *frame.location();
      fn.name = frame.name();

      pt::FunctionDecl::InputBlock input;
      for (const auto& child : frame.body().children()) {
        if (const auto* param = dynamic_cast<const ParameterActor*>(child.get())) {
          input.parameters.push_back(param->parameter());
        } else if (const auto* ret = dynamic_cast<const ReturnActor*>(child.get())) {
          fn.output = pt::FunctionDecl::OutputBlock{ret->ret()};
        } else if (const auto* node = dynamic_cast<const NodeActor*>(child.get())) {
          fn.body.nodes.emplace(node->portId(), node->node());
        } else if (const auto* conduit = dynamic_cast<const ConduitActor*>(child.get())) {
          pt::Conduit value = conduit->conduit();
          fn.body.conduits.emplace(value.id, std::move(value));
        }
      }
      if (!input.parameters.empty()) {
        fn.input = std::move(input);
      }
      return fn;
    }

  }  // namespace

  pt::ParseTree sceneToParseTree(const GraphScene& scene, const pt::Header& header) {
    pt::ParseTree tree;
    tree.header = header;
    for (const auto& child : scene.root().children()) {
      if (const auto* frame = dynamic_cast<const FunctionDeclActor*>(child.get())) {
        tree.declarations.emplace(frame->functionId(), pt::Declaration{functionOf(*frame)});
      }
    }
    return tree;
  }

}  // namespace fluir::editor
