#include "editor/transaction/delete_port.hpp"

#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "editor/core/tree_edit.hpp"

namespace fluir::editor {

  namespace {

    constexpr std::array BRANCHES{THEN_BRANCH_ID, ELSE_BRANCH_ID};

    et::Conditional* conditionalAt(et::ParseTree& tree, const fluir::FullID& path) {
      return std::get_if<et::Conditional>(nodeAt(tree, path));
    }

    std::vector<et::BlockPort>& portsOn(et::Conditional& conditional, PortRef ref) {
      return ref.output ? conditional.outputs : conditional.inputs;
    }

    // Position in `inputs`/`outputs`; inputs start after the condition.
    std::ptrdiff_t listPos(PortRef ref) { return static_cast<std::ptrdiff_t>(ref.output ? ref.index : ref.index - 1); }

    // Drops the wire ends at `index` on the conditional's outer side and shifts later ones down.
    void detachOuter(et::Block& block, fluir::ID cond, PortRef ref) {
      const auto index = static_cast<int>(ref.index);
      if (ref.output) {
        std::erase_if(block.conduits,
                      [&](const auto& entry) { return entry.second.input == cond && entry.second.index == index; });
        for (auto& [id, conduit] : block.conduits) {
          if (conduit.input == cond && conduit.index > index) {
            --conduit.index;
          }
        }
        return;
      }
      std::erase_if(block.conduits, [&](auto& entry) {
        auto& children = entry.second.children;
        const auto removed = std::erase_if(
          children, [&](const et::Conduit::Output& out) { return out.target == cond && out.index == index; });
        for (et::Conduit::Output& out : children) {
          if (out.target == cond && out.index > index) {
            --out.index;
          }
        }
        return removed > 0 && children.empty();
      });
    }

  }  // namespace

  bool DeletePort::execute(et::ParseTree& tree) {
    et::Conditional* conditional = conditionalAt(tree, path_);
    et::Block* parent = blockOf(tree, parentOf(path_));
    const bool isCondition = !ref_.output && ref_.index == 0;
    const et::BlockPort* port = conditional ? portOf(*conditional, ref_) : nullptr;
    if (!parent || !port || isCondition) {
      return false;
    }
    const fluir::ID cond = path_.back();
    const fluir::ID innerId = port->innerId;
    port_ = *port;

    outer_.clear();
    for (const auto& [id, conduit] : parent->conduits) {
      if (touches(conduit, cond)) {
        outer_.push_back(conduit);
      }
    }
    for (std::size_t i = 0; i < BRANCHES.size(); ++i) {
      branches_[i].clear();
      for (const auto& [id, conduit] : branchBlock(*conditional, BRANCHES[i])->conduits) {
        if (touches(conduit, innerId)) {
          branches_[i].push_back(conduit);
        }
      }
    }

    std::vector<et::BlockPort>& ports = portsOn(*conditional, ref_);
    ports.erase(ports.begin() + listPos(ref_));
    detachOuter(*parent, cond, ref_);
    for (const fluir::ID branchId : BRANCHES) {
      detachConduits(*branchBlock(*conditional, branchId), innerId);
    }
    return true;
  }

  bool DeletePort::unexecute(et::ParseTree& tree) {
    et::Conditional* conditional = conditionalAt(tree, path_);
    et::Block* parent = blockOf(tree, parentOf(path_));
    if (conditional == nullptr || parent == nullptr || !port_) {
      return false;
    }
    std::vector<et::BlockPort>& ports = portsOn(*conditional, ref_);
    if (listPos(ref_) > std::ssize(ports)) {
      return false;
    }
    ports.insert(ports.begin() + listPos(ref_), std::move(*port_));
    port_.reset();
    for (et::Conduit& conduit : outer_) {
      parent->conduits.insert_or_assign(conduit.id, std::move(conduit));
    }
    outer_.clear();
    for (std::size_t i = 0; i < BRANCHES.size(); ++i) {
      et::Block& branch = *branchBlock(*conditional, BRANCHES[i]);
      for (et::Conduit& conduit : branches_[i]) {
        branch.conduits.insert_or_assign(conduit.id, std::move(conduit));
      }
      branches_[i].clear();
    }
    return true;
  }

  std::unique_ptr<Transaction> deletePort(fluir::FullID path, PortRef ref) {
    return std::make_unique<DeletePort>(std::move(path), ref);
  }

}  // namespace fluir::editor
