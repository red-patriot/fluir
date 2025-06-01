#include "compiler/frontend/asg_builder.hpp"

#include <algorithm>
#include <ranges>
#include <unordered_set>
#include <variant>

namespace {
  std::unordered_set<fluir::ID> getTopLevelNodes(const fluir::pt::Block& block) {
    std::unordered_set<fluir::ID> topLevelNodes;

    // Start with all Nodes in the block
    std::ranges::transform(
      block.nodes, std::inserter(topLevelNodes, topLevelNodes.begin()), [](const auto& pair) { return pair.first; });

    // Remove all Nodes that are dependencies of other Nodes
    for (const auto& conduit : block.conduits | std::views::values) {
      topLevelNodes.erase(conduit.input);
    }

    return topLevelNodes;
  }
}  // namespace

namespace fluir {
  Results<asg::ASG> buildGraph(const pt::ParseTree& tree) { return ASGBuilder::buildFrom(tree); }

  Results<asg::ASG> ASGBuilder::buildFrom(const pt::ParseTree& tree) {
    ASGBuilder builder{tree};
    return builder.run();
  }

  ASGBuilder::ASGBuilder(const pt::ParseTree& tree) : tree_(tree) { }

  fluir::asg::Declaration ASGBuilder::operator()(const fluir::pt::FunctionDecl& func) {
    fluir::asg::FunctionDecl decl{func.id, func.location, func.name, {}};

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

    return Results<asg::ASG>{std::move(graph_), std::move(diagnostics_)};
  }

  Results<asg::DataFlowGraph> buildDataFlowGraph(pt::Block block) {
    return FlowGraphBuilder::buildFrom(std::move(block));
  }

  Results<asg::DataFlowGraph> FlowGraphBuilder::buildFrom(pt::Block block) {
    FlowGraphBuilder builder{std::move(block)};

    return builder.run();
  }

  FlowGraphBuilder::FlowGraphBuilder(pt::Block block) : block_(std::move(block)) { }

  fluir::asg::Node FlowGraphBuilder::operator()(const pt::Binary& pt) {
    inProgressNodes_.emplace_back(pt.id);
    fluir::asg::BinaryOp asg{pt.id, pt.location, pt.op, nullptr, nullptr};

    asg.lhs = getDependency(pt.id, 0);
    asg.rhs = getDependency(pt.id, 1);

    block_.nodes.erase(pt.id);
    inProgressNodes_.pop_back();
    return asg;
  }

  asg::Node FlowGraphBuilder::operator()(const pt::Unary& pt) {
    inProgressNodes_.emplace_back(pt.id);

    asg::UnaryOp asg{pt.id, pt.location, pt.op, nullptr};
    asg.operand = getDependency(pt.id, 0);

    block_.nodes.erase(pt.id);
    inProgressNodes_.pop_back();
    return asg;
  }

  asg::Node FlowGraphBuilder::operator()(const pt::Constant& pt) {
    inProgressNodes_.emplace_back(pt.id);

    asg::ConstantFP asg{pt.id, pt.location, pt.value};
    block_.nodes.erase(pt.id);
    // TODO: handle other literal types here

    inProgressNodes_.pop_back();
    return asg;
  }

  Results<asg::DataFlowGraph> FlowGraphBuilder::run() {
    // Find a Node without dependents in the graph
    auto topLevelNodes = getTopLevelNodes(block_);
    if (topLevelNodes.empty() && !block_.nodes.empty()) {
      // There is a circular dependency in the nodes, none of them are top-level
      // TODO: Detect which nodes form the cycle
      diagnostics_.emitError("Circular dependency detected.");
      return Results<asg::DataFlowGraph>{std::move(diagnostics_)};
    }

    for (const auto& ptNode : topLevelNodes) {
      auto asgNode = std::visit(*this, block_.nodes.at(ptNode));
      graph_.emplace_back(std::move(asgNode));
    }

    return {std::move(graph_), std::move(diagnostics_)};
  }

  asg::SharedDependency FlowGraphBuilder::getDependency(ID dependentId, int index) {
    // Find the dependency of ID:index in the graph
    const auto dependencyPt =
      std::ranges::find_if(block_.conduits, [&dependentId, &index](const pt::Block::Conduits::value_type& v) {
        auto& [_, conduit] = v;
        return std::ranges::any_of(conduit.children, [&dependentId, &index](const pt::Conduit::Output& out) {
          return out.target == dependentId && out.index == index;
        });
      });
    const auto& dependencyId = dependencyPt->second.input;

    // Check that the dependency index isn't already in progress
    if (std::ranges::find(inProgressNodes_, dependencyId) != inProgressNodes_.end()) {
      // We are trying to place a dependency on an in progress node, so
      // there is a circular dependency
      // TODO: Detect which nodes form the cycle
      diagnostics_.emitError("Circular dependency detected.");
      return nullptr;
    }

    if (alreadyFound_.contains(dependencyId)) {
      return alreadyFound_.at(dependencyId);
    }
    auto& pt = block_.nodes.at(dependencyId);  // TODO: Handle missing ID
    auto dependency = std::make_shared<fluir::asg::Node>(std::visit(*this, pt));
    alreadyFound_.insert({dependencyId, dependency});
    return dependency;
  }
}  // namespace fluir
