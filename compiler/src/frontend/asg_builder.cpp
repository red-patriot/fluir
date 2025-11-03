#include "compiler/frontend/asg_builder.hpp"

#include <algorithm>
#include <ranges>
#include <unordered_set>
#include <variant>

#include "compiler/scope_guard.hpp"

namespace {
  /** Returns the set of Nodes that are not dependencies of other Nodes */
  std::unordered_set<fluir::ID> getSinkNodes(const fluir::pt::Block& block) {
    std::unordered_set<fluir::ID> sinkNodes;

    // Start with all Nodes in the block
    std::ranges::transform(
      block.nodes, std::inserter(sinkNodes, sinkNodes.begin()), [](const auto& pair) { return pair.first; });

    // Remove all Nodes that are dependencies of other Nodes
    std::ranges::for_each(block.conduits | std::views::values,
                          [&sinkNodes](const auto& conduit) { sinkNodes.erase(conduit.input); });

    return sinkNodes;
  }
}  // namespace

namespace fluir {
  Results<asg::ASG> buildGraph(Context& ctx, const pt::ParseTree& tree) { return ASGBuilder::buildFrom(ctx, tree); }

  Results<asg::ASG> ASGBuilder::buildFrom(Context& ctx, const pt::ParseTree& tree) {
    ASGBuilder builder{ctx, tree};
    return builder.run();
  }

  ASGBuilder::ASGBuilder(Context& ctx, const pt::ParseTree& tree) : ctx_(ctx), tree_(tree) { }

  fluir::asg::Declaration ASGBuilder::operator()(const fluir::pt::FunctionDecl& func) {
    fluir::asg::FunctionDecl decl{func.id, func.location, func.name, {}};

    auto bodyResults = buildDataFlowGraph(ctx_, func.body);

    if (!ctx_.diagnostics.containsErrors()) {
      decl.statements = std::move(bodyResults.value());
    }

    return decl;
  }

  Results<asg::ASG> ASGBuilder::run() {
    for (const auto& declaration : tree_.declarations | std::views::values) {
      graph_.declarations.emplace_back(std::visit(*this, declaration));
    }
    if (ctx_.diagnostics.containsErrors()) {
      return NoResult;
    }

    return std::move(graph_);
  }

  Results<asg::DataFlowGraph> buildDataFlowGraph(Context& ctx, pt::Block block) {
    return FlowGraphBuilder::buildFrom(ctx, std::move(block));
  }

  Results<asg::DataFlowGraph> FlowGraphBuilder::buildFrom(Context& ctx, pt::Block block) {
    FlowGraphBuilder builder{ctx, std::move(block)};

    return builder.run();
  }

  FlowGraphBuilder::FlowGraphBuilder(Context& ctx, pt::Block block) : ctx_(ctx), block_(std::move(block)) { }

  asg::UniqueNode FlowGraphBuilder::operator()(const pt::Binary& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };
    fluir::asg::BinaryOp asg{{pt.id, pt.location}, pt.op, nullptr, nullptr};

    asg.lhs = getDependency(pt.id, 0);
    asg.rhs = getDependency(pt.id, 1);

    return std::make_unique<asg::Node>(asg);
  }

  asg::UniqueNode FlowGraphBuilder::operator()(const pt::Unary& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };

    asg::UnaryOp asg{{pt.id, pt.location}, pt.op, nullptr};
    asg.operand = getDependency(pt.id, 0);

    return std::make_unique<asg::Node>(asg);
  }

  asg::UniqueNode FlowGraphBuilder::operator()(const pt::Constant& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };

    asg::ConstantFP asg{{pt.id, pt.location}, pt.value};
    // TODO: handle other literal types here

    return std::make_unique<asg::Node>(asg);
  }

  Results<asg::DataFlowGraph> FlowGraphBuilder::run() {
    // Find a Node without dependents in the graph
    const auto sinkNodes = getSinkNodes(block_);
    if (sinkNodes.empty() && !block_.nodes.empty()) {
      // There is a circular dependency in the nodes, none of them are top-level
      // TODO: Detect which nodes form the cycle
      ctx_.diagnostics.emitError("Circular dependency detected.");
      return NoResult;
    }

    for (const auto& ptNode : sinkNodes) {
      auto asgNode = std::visit(*this, block_.nodes.at(ptNode));
      graph_.emplace_back(std::move(asgNode));
    }

    return std::move(graph_);
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
      ctx_.diagnostics.emitError("Circular dependency detected.");
      return nullptr;
    }

    if (alreadyFound_.contains(dependencyId)) {
      return alreadyFound_.at(dependencyId);
    }
    auto& pt = block_.nodes.at(dependencyId);  // TODO: Handle missing ID
    asg::SharedDependency dependency{std::visit(*this, pt)};
    alreadyFound_.insert({dependencyId, dependency});
    return dependency;
  }
}  // namespace fluir
