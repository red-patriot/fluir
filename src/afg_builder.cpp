#include "fluir/compiler/afg_builder.hpp"

#include <algorithm>
#include <unordered_set>

namespace {
  struct DependencyLister {
   public:
    std::unordered_set<fluir::id_t> idsInTree() const { return std::move(deps_); }

    void operator()(const fluir::parse_tree::Constant& n) { }
    void operator()(const fluir::parse_tree::Unary& n) {
      deps_.insert(n.lhs);
    }
    void operator()(const fluir::parse_tree::Binary& n) {
      deps_.insert(n.lhs);
      deps_.insert(n.rhs);
    }

   private:
    std::unordered_set<fluir::id_t> deps_;
  };

  struct AfgBuilder {
   public:
    AfgBuilder(fluir::parse_tree::Block& block,
               std::unordered_map<fluir::id_t, fluir::afg::Dependency>& alreadyFound) :
        block_(block),
        alreadyFound_(alreadyFound) { }

    fluir::afg::Node* operator()(const fluir::parse_tree::Constant& n) {
      return nullptr;
    }
    fluir::afg::Node* operator()(const fluir::parse_tree::Unary& n) {
      return nullptr;
    }
    fluir::afg::Node* operator()(const fluir::parse_tree::Binary& n) {
      // TODO look in already found
      auto& lhsElement = block_.nodes.at(n.lhs);
      fluir::afg::Dependency lhs = fluir::afg::Dependency{std::visit(*this, lhsElement)};
      alreadyFound_.insert({n.lhs, lhs});

      auto& rhsElement = block_.nodes.at(n.rhs);
      fluir::afg::Dependency rhs = fluir::afg::Dependency{std::visit(*this, rhsElement)};
      alreadyFound_.insert({n.rhs, rhs});

      // Remove LHS and RHS from the block
      block_.nodes.erase(n.lhs);
      block_.nodes.erase(n.rhs);
      block_.nodes.erase(n.id);

      return new fluir::afg::BinaryOp{n.op,
                                      lhs,
                                      rhs,
                                      n.location};
    }

   private:
    fluir::parse_tree::Block& block_;
    std::unordered_map<fluir::id_t, fluir::afg::Dependency>& alreadyFound_;
  };
}  // namespace

namespace fluir::compiler {
  afg::FlowGraph buildGraphOf(parse_tree::Block block) {
    afg::FlowGraph graph;

    std::unordered_map<id_t, afg::Dependency> foundDeps;

    // Find a top-level node
    std::unordered_set<id_t> stillInTree;
    {
      DependencyLister v;
      for (const auto& node : block.nodes) {
        std::visit(v, node.second);
      }

      stillInTree = v.idsInTree();
    }

    auto topLevel = std::find_if_not(
        block.nodes.begin(), block.nodes.end(),
        [&stillInTree](const fluir::parse_tree::Nodes::value_type& v) {
          return stillInTree.contains(v.first);
        });
    if (topLevel == block.nodes.end()) {
      // TODO: Handle this case
      return graph;
    }
    auto [id, next] = *topLevel;

    // Make an AFG node, find its dependencies in the parse tree
    auto node = std::unique_ptr<afg::Node>(std::visit(AfgBuilder{block, foundDeps}, next));
    graph.add(std::move(node));

    return graph;
  }
}  // namespace fluir::compiler
