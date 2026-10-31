#include "editor/diagram/ids.hpp"

#include <algorithm>

#include "editor/diagram/conditional.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/node.hpp"

namespace fluir::editor::diagram {
  namespace {

    fluir::ID largestIn(const QGraphicsItem& scope) {
      fluir::ID largest = fluir::INVALID_ID;
      for (const QGraphicsItem* child : scope.childItems()) {
        if (const auto* node = dynamic_cast<const FluirNode*>(child)) {
          largest = std::max(largest, node->id());
        } else if (const auto* conduit = dynamic_cast<const Conduit*>(child)) {
          largest = std::max(largest, conduit->id());
        }
      }
      return largest;
    }

  }  // namespace

  fluir::ID nextId(const Scope& scope) {
    fluir::ID largest = fluir::INVALID_ID;
    if (const auto* fn = dynamic_cast<const Function*>(scope.parentItem())) {
      largest = largestIn(scope);
      const draw::Signature& signature = fn->signature();
      if (signature.input) {
        for (const pt::FunctionParameter& param : signature.input->parameters) {
          largest = std::max(largest, param.id);
        }
      }
      if (signature.output && signature.output->ret) {
        largest = std::max(largest, signature.output->ret->id);
      }
    } else {
      // A branch shares its id space with its sibling branch and its conditional's ports.
      const auto& conditional = static_cast<const Conditional&>(*scope.parentItem());
      largest = std::max(largestIn(*conditional.thenBranch()), largestIn(*conditional.elseBranch()));
      for (const draw::PortRef ref : draw::portRefs(conditional.ports())) {
        largest = std::max(largest, draw::portOf(conditional.ports(), ref)->innerId);
      }
    }
    return largest + 1;
  }

}  // namespace fluir::editor::diagram
