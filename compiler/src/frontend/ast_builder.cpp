#include "compiler/frontend/ast_builder.hpp"

#include <algorithm>
#include <unordered_set>
#include <variant>

namespace {
  class GraphBuilder {
   public:
    fluir::ast::Declaration operator()(const fluir::pt::FunctionDecl& func) {
      return fluir::ast::FunctionDecl{func.id,
                                      func.location,
                                      func.name,
                                      {}};  // TODO: Handle body
    }
  };

  class DependencyWalker {
   public:
    std::unordered_set<fluir::ID> idsInTree() const { return std::move(deps_); }

    void operator()(const fluir::pt::Constant& n) { }
    void operator()(const fluir::pt::Unary& n) {
      deps_.insert(n.lhs);
    }
    void operator()(const fluir::pt::Binary& n) {
      deps_.insert(n.lhs);
      deps_.insert(n.rhs);
    }

   private:
    std::unordered_set<fluir::ID> deps_;
  };

  class AFGBuilder {
   public:
    AFGBuilder(fluir::pt::Block& block,
               std::unordered_map<fluir::ID, fluir::ast::SharedDependency>& alreadyFound) :
        block_(block),
        alreadyFound_(alreadyFound) { }

    fluir::ast::Node operator()(const fluir::pt::Binary& pt) {
      fluir::ast::BinaryOp ast{
          pt.id,
          pt.location,
          pt.op,
          nullptr, nullptr};

      block_.erase(pt.id);

      ast.lhs = getDependency(pt.lhs);
      ast.rhs = getDependency(pt.rhs);

      return ast;
    }

    fluir::ast::Node operator()(const fluir::pt::Unary& pt) {
      fluir::ast::UnaryOp ast{pt.id,
                              pt.location,
                              pt.op,
                              nullptr};
      ast.operand = getDependency(pt.lhs);

      block_.erase(pt.id);
      return ast;
    }

    fluir::ast::Node operator()(const fluir::pt::Constant& pt) {
      fluir::ast::ConstantFP ast{pt.id, pt.location, pt.value};
      block_.erase(pt.id);
      // TODO: handle other literal types here
      return ast;
    }

   private:
    fluir::pt::Block& block_;
    std::unordered_map<fluir::ID, fluir::ast::SharedDependency>& alreadyFound_;

    fluir::ast::SharedDependency getDependency(fluir::ID id) {
      if (alreadyFound_.contains(id)) {
        return alreadyFound_.at(id);
      }
      auto& pt = block_.at(id);  // TODO: Handle missing ID
      auto dependency = std::make_shared<fluir::ast::Node>(std::visit(*this, pt));
      alreadyFound_.insert({id, dependency});
      return dependency;
    }
  };
}  // namespace

namespace fluir {
  Results<ast::ASG> buildGraph(const pt::ParseTree& tree) {
    ast::ASG graph;
    Diagnostics diagnostics;

    for (const auto& [id, declaration] : tree.declarations) {
      graph.declarations.emplace_back(std::visit(GraphBuilder{}, declaration));
    }

    return Results<ast::ASG>{graph, diagnostics};
  }

  Results<ast::DataFlowGraph> buildDataFlowGraph(pt::Block block) {
    ast::DataFlowGraph graph;
    Diagnostics diagnostics;

    // Find a Node without dependents in the graph
    std::unordered_map<ID, ast::SharedDependency> found;  // Dependencies that have been found so far
    while (!block.empty() /*  && noTopLevelCycles()? */) {
      std::unordered_set<ID> treeDependencies;
      {
        DependencyWalker v;
        for (const auto& node : block) {
          std::visit(v, node.second);
        }

        treeDependencies = v.idsInTree();
      }

      auto topLevelNode = std::ranges::find_if_not(
          block,
          [&treeDependencies](const pt::Block::value_type& v) {
            auto& [key, value] = v;
            return treeDependencies.contains(key);
          });
      if (topLevelNode == block.end()) {
        // TODO: Handle this
        return Results<ast::DataFlowGraph>{Diagnostics{{Diagnostic::ERROR, "Unimplemented"}}};
      }
      auto ptNode = block.extract(topLevelNode);
      // Build ASG node from PT node
      auto asgNode = std::visit(AFGBuilder{block, found}, ptNode.mapped());

      // Output ASG node
      graph.emplace_back(std::move(asgNode));
    }

    return {std::move(graph), std::move(diagnostics)};
  }
}  // namespace fluir
