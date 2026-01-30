#include "compiler/frontend/ast_builder.hpp"

#include <algorithm>
#include <ranges>
#include <unordered_set>
#include <variant>

#include "../../include/compiler/utility/scope_guard.hpp"

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
  Results<ast::AST> buildGraph(Context& ctx, const pt::ParseTree& tree) { return ASTBuilder::buildFrom(ctx, tree); }

  Results<ast::AST> ASTBuilder::buildFrom(Context& ctx, const pt::ParseTree& tree) {
    ASTBuilder builder{ctx, tree};
    return builder.run();
  }

  ASTBuilder::ASTBuilder(Context& ctx, const pt::ParseTree& tree) : ctx_(ctx), tree_(tree) { }

  Results<ast::Declaration> ASTBuilder::operator()(const fluir::pt::FunctionDecl& func) {
    fluir::ast::FunctionDecl decl{func.id, func.location, func.name, {}};

    auto bodyResults = buildDataFlowGraph(ctx_, func.body, {func.id});

    if (!bodyResults) {
      return NoResult;
    }
    decl.statements = std::move(bodyResults.value());

    return decl;
  }

  Results<ast::AST> ASTBuilder::run() {
    try {
      bool failed = false;
      for (const auto& declaration : tree_.declarations | std::views::values) {
        auto declAst = std::visit(*this, declaration);
        if (declAst.has_value()) {
          graph_.declarations.emplace_back(std::move(declAst.value()));
        } else {
          failed = true;
        }
      }
      if (failed) {
        return NoResult;
      }
    } catch (const diagnostic::Panic&) {
      return NoResult;
    }

    return std::move(graph_);
  }

  Results<ast::DataFlowGraph> buildDataFlowGraph(Context& ctx, pt::Block block, std::vector<ID> parents) {
    return FlowGraphBuilder::buildFrom(ctx, std::move(block), std::move(parents));
  }

  Results<ast::DataFlowGraph> FlowGraphBuilder::buildFrom(Context& ctx, pt::Block block, std::vector<ID> parents) {
    FlowGraphBuilder builder{ctx, std::move(block), std::move(parents)};

    return builder.run();
  }

  FlowGraphBuilder::FlowGraphBuilder(Context& ctx, pt::Block block, std::vector<ID> parents) :
    ctx_(ctx), block_(std::move(block)), parents_(std::move(parents)) { }

  ast::UniqueNode FlowGraphBuilder::operator()(const pt::Binary& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };

    parents_.push_back(pt.id);
    FLUIR_SCOPE_EXIT { parents_.pop_back(); };
    return std::make_unique<ast::BinaryOp>(
      pt.op, getDependency(pt.id, 0), getDependency(pt.id, 1), parents_, pt.location);
  }

  ast::UniqueNode FlowGraphBuilder::operator()(const pt::Unary& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };
    parents_.push_back(pt.id);
    FLUIR_SCOPE_EXIT { parents_.pop_back(); };

    return std::make_unique<ast::UnaryOp>(pt.op, getDependency(pt.id, 0), parents_, pt.location);
  };

  ast::UniqueNode FlowGraphBuilder::operator()(const pt::Constant& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };
    parents_.push_back(pt.id);
    FLUIR_SCOPE_EXIT { parents_.pop_back(); };
    return std::make_unique<ast::Constant>(pt.value, parents_, pt.location);
  }

  Results<ast::DataFlowGraph> FlowGraphBuilder::run() {
    // Find a Node without dependents in the graph
    const auto sinkNodes = getSinkNodes(block_);
    if (sinkNodes.empty() && !block_.nodes.empty()) {
      // There is a circular dependency in the nodes, none of them are top-level
      // TODO: Detect which nodes form the cycle
      ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_CIRCULAR_DEPENDENCY, ctx_.currentFile, {});
    }

    for (const auto& ptNode : sinkNodes) {
      auto astNode = std::visit(*this, block_.nodes.at(ptNode));
      graph_.emplace_back(std::move(astNode));
    }

    return std::move(graph_);
  }

  ast::SharedDependency FlowGraphBuilder::getDependency(ID dependentId, int index) {
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
      ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_CIRCULAR_DEPENDENCY, ctx_.currentFile, {});
    }

    if (alreadyFound_.contains(dependencyId)) {
      return alreadyFound_.at(dependencyId);
    }
    auto& pt = block_.nodes.at(dependencyId);  // TODO: Handle missing ID
    ast::SharedDependency dependency{std::visit(*this, pt)};
    alreadyFound_.insert({dependencyId, dependency});
    return dependency;
  }
}  // namespace fluir
