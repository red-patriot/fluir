#include "compiler/frontend/ast_builder.hpp"

#include <algorithm>
#include <unordered_set>
#include <variant>

namespace {
  class DependencyWalker {
   public:
    static std::unordered_set<fluir::ID> getAll(const fluir::pt::Block& block) {
      DependencyWalker v;
      for (const auto& node : block) {
        std::visit(v, node.second);
      }

      return v.idsInTree();
    }

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
}  // namespace

namespace fluir {
  Results<ast::ASG> buildGraph(const pt::ParseTree& tree) {
    return ASGBuilder::buildFrom(tree);
  }

  Results<ast::ASG> ASGBuilder::buildFrom(const pt::ParseTree& tree) {
    ASGBuilder builder{tree};
    return builder.run();
  }

  ASGBuilder::ASGBuilder(const pt::ParseTree& tree) :
      tree_(tree) { }

  fluir::ast::Declaration ASGBuilder::operator()(const fluir::pt::FunctionDecl& func) {
    fluir::ast::FunctionDecl decl{func.id,
                                  func.location,
                                  func.name,
                                  {}};

    auto bodyResults = buildDataFlowGraph(func.body);
    // TODO: Handle errors in body
    decl.statements = std::move(bodyResults.value());

    return decl;
  }

  Results<ast::ASG> ASGBuilder::run() {
    for (const auto& [id, declaration] : tree_.declarations) {
      graph_.declarations.emplace_back(std::visit(*this, declaration));
    }

    return Results<ast::ASG>{std::move(graph_),
                             std::move(diagnostics_)};
  }

  Results<ast::DataFlowGraph> buildDataFlowGraph(pt::Block block) {
    return FlowGraphBuilder::buildFrom(std::move(block));
  }

  Results<ast::DataFlowGraph> FlowGraphBuilder::buildFrom(pt::Block block) {
    FlowGraphBuilder builder{std::move(block)};

    return builder.run();
  }

  FlowGraphBuilder::FlowGraphBuilder(pt::Block block) :
      block_(std::move(block)) { }

  fluir::ast::Node FlowGraphBuilder::operator()(const pt::Binary& pt) {
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

  ast::Node FlowGraphBuilder::operator()(const pt::Unary& pt) {
    ast::UnaryOp ast{pt.id,
                     pt.location,
                     pt.op,
                     nullptr};
    ast.operand = getDependency(pt.lhs);

    block_.erase(pt.id);
    return ast;
  }

  ast::Node FlowGraphBuilder::operator()(const pt::Constant& pt) {
    ast::ConstantFP ast{pt.id, pt.location, pt.value};
    block_.erase(pt.id);
    // TODO: handle other literal types here
    return ast;
  }

  Results<ast::DataFlowGraph> FlowGraphBuilder::run() {
    // Find a Node without dependents in the graph
    while (!block_.empty() /*  && noTopLevelCycles()? */) {
      auto treeDependencies = DependencyWalker::getAll(block_);

      auto topLevelNode = std::ranges::find_if_not(
          block_,
          [&treeDependencies](const pt::Block::value_type& v) {
            auto& [key, value] = v;
            return treeDependencies.contains(key);
          });
      if (topLevelNode == block_.end()) {
        // TODO: Handle this
        return Results<ast::DataFlowGraph>{Diagnostics{{Diagnostic::ERROR, "Unimplemented"}}};
      }
      auto ptNode = block_.extract(topLevelNode);
      // Build ASG node from PT node
      auto asgNode = std::visit(*this, ptNode.mapped());

      // Output ASG node
      graph_.emplace_back(std::move(asgNode));
    }

    return {std::move(graph_), std::move(diagnostics_)};
  }

  ast::SharedDependency FlowGraphBuilder::getDependency(ID id) {
    if (alreadyFound_.contains(id)) {
      return alreadyFound_.at(id);
    }
    auto& pt = block_.at(id);  // TODO: Handle missing ID
    auto dependency = std::make_shared<fluir::ast::Node>(std::visit(*this, pt));
    alreadyFound_.insert({id, dependency});
    return dependency;
  }
}  // namespace fluir
