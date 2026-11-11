#include "editor/diagram/export.hpp"

#include "editor/diagram/binary.hpp"
#include "editor/diagram/call.hpp"
#include "editor/diagram/comment.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/constant.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/parameter.hpp"
#include "editor/diagram/port.hpp"
#include "editor/diagram/return.hpp"
#include "editor/diagram/terminal.hpp"
#include "editor/diagram/unary.hpp"

namespace fluir::editor::diagram {
  namespace {

    fluir::ID endpointOf(const Terminal& terminal) {
      const QGraphicsItem* owner = terminal.parentItem();
      if (const auto* node = dynamic_cast<const FluirNode*>(owner)) {
        return node->id();
      }
      if (const auto* param = dynamic_cast<const Parameter*>(owner)) {
        return param->id();
      }
      if (const auto* ret = dynamic_cast<const Return*>(owner)) {
        return ret->id();
      }
      const auto& port = dynamic_cast<const Port&>(*owner);
      return &terminal == port.outer() ? dynamic_cast<const Conditional&>(*port.parentItem()).id() : port.innerId();
    }

    pt::Conduit conduitOf(const Conduit& conduit) {
      pt::Conduit out{
        .id = conduit.id(), .input = endpointOf(*conduit.source()), .index = conduit.source()->index(), .children = {}};
      for (const Terminal* sink : conduit.sinks()) {
        out.children.push_back({.target = endpointOf(*sink), .index = sink->index()});
      }
      return out;
    }

    pt::Block blockOf(const QGraphicsItem& scope);

    pt::Node nodeOf(const FluirNode& node) {
      if (const auto* n = dynamic_cast<const Constant*>(&node)) {
        return n->data();
      }
      if (const auto* n = dynamic_cast<const Binary*>(&node)) {
        return n->data();
      }
      if (const auto* n = dynamic_cast<const Unary*>(&node)) {
        return n->data();
      }
      if (const auto* n = dynamic_cast<const Call*>(&node)) {
        return n->data();
      }
      if (const auto* n = dynamic_cast<const Comment*>(&node)) {
        return n->data();
      }
      const auto& conditional = dynamic_cast<const Conditional&>(node);
      return pt::Conditional{.id = conditional.id(),
                             .location = conditional.location(),
                             .condition = conditional.ports().condition,
                             .inputs = conditional.ports().inputs,
                             .outputs = conditional.ports().outputs,
                             .thenScope = xyz::indirect<pt::Block>(blockOf(*conditional.thenBranch())),
                             .elseScope = xyz::indirect<pt::Block>(blockOf(*conditional.elseBranch()))};
    }

    pt::Block blockOf(const QGraphicsItem& scope) {
      pt::Block block;
      for (const QGraphicsItem* child : scope.childItems()) {
        if (const auto* node = dynamic_cast<const FluirNode*>(child)) {
          block.nodes.emplace(node->id(), nodeOf(*node));
        } else if (const auto* conduit = dynamic_cast<const Conduit*>(child)) {
          block.conduits.emplace(conduit->id(), conduitOf(*conduit));
        }
      }
      return block;
    }

    pt::FunctionDecl functionOf(const Function& fn) {
      return pt::FunctionDecl{.id = fn.id(),
                              .location = fn.location(),
                              .name = fn.signature().name,
                              .body = blockOf(*fn.body()),
                              .input = fn.signature().input,
                              .output = fn.signature().output};
    }

  }  // namespace

  pt::ParseTree exportTree(const ModuleScene& scene) {
    pt::ParseTree tree{.header = scene.header(), .declarations = {}};
    for (const QGraphicsItem* item : scene.items()) {
      if (item->parentItem() != nullptr) {
        continue;
      }
      if (const auto* fn = dynamic_cast<const Function*>(item)) {
        tree.declarations.emplace(fn->id(), functionOf(*fn));
      } else if (const auto* comment = dynamic_cast<const Comment*>(item)) {
        tree.declarations.emplace(comment->id(), comment->data());
      }
    }
    return tree;
  }

}  // namespace fluir::editor::diagram
