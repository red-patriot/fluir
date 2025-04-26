#include "compiler/frontend/asg_builder.hpp"

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
  Results<asg::ASG> buildGraph(const pt::ParseTree& tree) {
    return ASGBuilder::buildFrom(tree);
  }

  Results<asg::ASG> ASGBuilder::buildFrom(const pt::ParseTree& tree) {
    ASGBuilder builder{tree};
    return builder.run();
  }

  ASGBuilder::ASGBuilder(const pt::ParseTree& tree) :
      tree_(tree) { }

  fluir::asg::Declaration ASGBuilder::operator()(const fluir::pt::FunctionDecl& func) {
    fluir::asg::FunctionDecl decl{func.id,
                                  func.location,
                                  func.name,
                                  {}};

    auto bodyResults = buildDataFlowGraph(func.body);
    std::ranges::move(bodyResults.diagnostics(), std::back_inserter(diagnostics_));

    if (!diagnostics_.containsErrors()) {
      decl.statements = std::move(bodyResults.value());
    }

    return decl;
  }

  Results<asg::ASG> ASGBuilder::run() {
    for (const auto& [id, declaration] : tree_.declarations) {
      graph_.declarations.emplace_back(std::visit(*this, declaration));
    }
    if (diagnostics_.containsErrors()) {
      return Results<asg::ASG>{std::move(diagnostics_)};
    }

    return Results<asg::ASG>{std::move(graph_),
                             std::move(diagnostics_)};
  }

  Results<asg::DataFlowGraph> buildDataFlowGraph(pt::Block block) {
    return FlowGraphBuilder::buildFrom(std::move(block));
  }

  Results<asg::DataFlowGraph> FlowGraphBuilder::buildFrom(pt::Block block) {
    FlowGraphBuilder builder{std::move(block)};

    return builder.run();
  }

  FlowGraphBuilder::FlowGraphBuilder(pt::Block block) :
      block_(std::move(block)) { }

  fluir::asg::Node FlowGraphBuilder::operator()(const pt::Binary& pt) {
    inProgressNodes_.emplace_back(pt.id);
    fluir::asg::BinaryOp asg{
        pt.id,
        pt.location,
        pt.op,
        nullptr, nullptr};

    asg.lhs = getDependency(pt.lhs);
    asg.rhs = getDependency(pt.rhs);

    block_.erase(pt.id);
    inProgressNodes_.pop_back();
    return asg;
  }

  asg::Node FlowGraphBuilder::operator()(const pt::Unary& pt) {
    inProgressNodes_.emplace_back(pt.id);

    asg::UnaryOp asg{pt.id,
                     pt.location,
                     pt.op,
                     nullptr};
    asg.operand = getDependency(pt.lhs);

    block_.erase(pt.id);
    inProgressNodes_.pop_back();
    return asg;
  }

  asg::Node FlowGraphBuilder::operator()(const pt::Constant& pt) {
    inProgressNodes_.emplace_back(pt.id);

    asg::ConstantFP asg{pt.id, pt.location, pt.value};
    block_.erase(pt.id);
    // TODO: handle other literal types here

    inProgressNodes_.pop_back();
    return asg;
  }

  Results<asg::DataFlowGraph> FlowGraphBuilder::run() {
    // Find a Node without dependents in the graph
    while (!block_.empty()) {
      auto treeDependencies = DependencyWalker::getAll(block_);

      auto topLevelNode = std::ranges::find_if_not(
          block_,
          [&treeDependencies](const pt::Block::value_type& v) {
            auto& [key, value] = v;
            return treeDependencies.contains(key);
          });
      if (topLevelNode == block_.end()) {
        // There is a circular dependency in the nodes.
        // TODO: Detect which nodes form the cycle
        diagnostics_.emitError("Circular dependency detected.");
        return Results<asg::DataFlowGraph>{std::move(diagnostics_)};
      }
      auto ptNode = block_.extract(topLevelNode);
      auto asgNode = std::visit(*this, ptNode.mapped());

      graph_.emplace_back(std::move(asgNode));
    }

    return {std::move(graph_), std::move(diagnostics_)};
  }

  asg::SharedDependency FlowGraphBuilder::getDependency(ID id) {
    if (std::ranges::find(inProgressNodes_, id) != inProgressNodes_.end()) {
      // We are trying to place a dependency on an in progress node, so
      // there is a circular dependency
      // TODO: Detect which nodes form the cycle
      diagnostics_.emitError("Circular dependency detected.");
      return nullptr;
    }
    if (alreadyFound_.contains(id)) {
      return alreadyFound_.at(id);
    }
    auto& pt = block_.at(id);  // TODO: Handle missing ID
    auto dependency = std::make_shared<fluir::asg::Node>(std::visit(*this, pt));
    alreadyFound_.insert({id, dependency});
    return dependency;
  }
}  // namespace fluir
