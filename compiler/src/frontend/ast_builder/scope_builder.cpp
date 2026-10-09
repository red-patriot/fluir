#include "compiler/frontend/ast_builder/scope_builder.hpp"

#include <algorithm>
#include <ranges>
#include <unordered_set>
#include <variant>

#include "compiler/utility/scope_guard.hpp"

namespace fluir::fe {
  Results<ast::DataFlowGraph> ScopeBuilder::buildAst(Context& ctx,
                                                     const pt::Block& block,
                                                     const std::unordered_set<ID>& inputs,
                                                     const std::unordered_set<ID>& outputs,
                                                     FullID id) {
    ScopeBuilder builder{ctx, block, inputs, outputs, std::move(id)};
    return builder.run();
  }

  ScopeBuilder::ScopeBuilder(Context& ctx,
                             pt::Block pt,
                             const std::unordered_set<ID>& inputs,
                             const std::unordered_set<ID>& outputs,
                             FullID id) :
    ctx_(ctx), pt_(std::move(pt)), currentID_(std::move(id)), inputs_(inputs), outputs_(outputs) { }

  ast::UniqueNode ScopeBuilder::operator()(const pt::Binary& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };
    return std::make_unique<ast::BinaryOp>(
      pt.op, getDependency(pt.id, 0), getDependency(pt.id, 1), currentID_, pt.location);
  }

  ast::UniqueNode ScopeBuilder::operator()(const pt::Unary& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };
    return std::make_unique<ast::UnaryOp>(pt.op, getDependency(pt.id, 0), currentID_, pt.location);
  };

  ast::UniqueNode ScopeBuilder::operator()(const pt::Constant& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };
    return std::make_unique<ast::Constant>(pt.value, currentID_, pt.location);
  }

  ast::UniqueNode ScopeBuilder::operator()(const pt::Call& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };

    auto ptArgs = pt.arguments;
    std::ranges::sort(
      ptArgs, [](const pt::Call::Argument& lhs, const pt::Call::Argument& rhs) { return lhs.index < rhs.index; });

    std::vector<ast::Dependency> astArgs;
    for (const auto& [name, index] : ptArgs) {
      auto argument = getDependency(pt.id, index);
      astArgs.emplace_back(std::move(argument));
    }

    return ast::createNode<ast::Call>(pt.target, std::move(astArgs), currentID_, pt.location);
  }

  ast::UniqueNode ScopeBuilder::operator()(const pt::Conditional& pt) {
    constexpr int CONDITION_INDEX = 0;
    constexpr ID THEN_BODY = 1;
    constexpr ID ELSE_BODY = 2;
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };

    auto condition = getDependency(pt.id, CONDITION_INDEX);
    std::unordered_set<ID> inputIds = {pt.condition.innerId};
    std::vector<ast::ScopeInput> inputs;
    for (const auto [idx, ptInput] : pt.inputs | std::ranges::views::enumerate) {
      inputs.emplace_back(getDependency(pt.id, idx + 1),  // Index 0 is the condition, so inc by 1 for the other inputs
                          ptInput.innerId);
      inputIds.insert(ptInput.innerId);
    }
    std::unordered_set<ID> outputIds;
    std::ranges::transform(
      pt.outputs, std::inserter(outputIds, outputIds.begin()), [](const auto& o) { return o.innerId; });

    ast::DataFlowGraph thenBody;
    {
      currentID_.emplace_back(THEN_BODY);
      FLUIR_SCOPE_EXIT { currentID_.pop_back(); };
      auto maybeThen = buildAst(ctx_, *pt.thenScope, inputIds, outputIds, currentID_);
      if (maybeThen) {
        thenBody = std::move(*maybeThen);
      }
    }
    ast::DataFlowGraph elseBody;
    {
      currentID_.emplace_back(ELSE_BODY);
      FLUIR_SCOPE_EXIT { currentID_.pop_back(); };
      auto maybeElse = buildAst(ctx_, *pt.elseScope, inputIds, outputIds, currentID_);
      if (maybeElse) {
        elseBody = std::move(*maybeElse);
      }
    }

    std::vector<ID> outputs;
    std::ranges::transform(pt.outputs, std::back_inserter(outputs), [](const auto& o) { return o.innerId; });

    return ast::createNode<ast::Conditional>(currentID_,
                                             pt.location,
                                             std::move(condition),
                                             std::move(inputs),
                                             std::move(outputs),
                                             std::move(thenBody),
                                             std::move(elseBody));
  }

  Results<ast::DataFlowGraph> ScopeBuilder::run() {
    const auto& body = pt_;
    alreadyFound_.reserve(body.nodes.size());

    // Process nodes that will become local variables so they are available for use
    promotedIds_ = getIdsPromotedToLocal();
    currentID_.push_back(INVALID_ID);  // Make a space for the next level of IDs
    dag::Arcs<ID> dependencies;
    for (const auto& local : promotedIds_) {
      currentID_.back() = local;
      auto astNode = process(local, body.nodes.at(local));
      if (!astNode) {
        continue;
      }
      alreadyFound_.insert(local);
      const auto location = astNode->location();
      auto write = ast::createNode<ast::LocalWrite>(ast::createDependency(std::nullopt, std::move(astNode)), location);
      dependencies.insert({local, std::move(dependencies_)});
      dependencies_ = {};
      graph_.emplace_back(std::move(write));
    }
    // Topological sort of graph_ so locals are written/read in the right order
    if (!dag::topologicalSort(graph_, dependencies, [](const ast::UniqueNode& node) -> ID { return node->id(); })) {
      ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_CIRCULAR_DEPENDENCY, ctx_.currentFile, {});
    }

    // Process outputs
    for (const auto& output : outputs_) {
      FullID outputID = currentID_;
      outputID.push_back(output);
      auto astNode = getDependency(output, std::nullopt);
      auto write =
        ast::createNode<ast::LocalWrite>(std::move(outputID), std::move(astNode), fluir::FlowGraphLocation{});
      graph_.emplace_back(std::move(write));
    }

    // If there are more pending nodes, process them now
    if (alreadyFound_.size() != body.nodes.size()) {
      // Find a Node without dependents in the graph
      const auto sinkNodes = getSinkIds();
      if (sinkNodes.empty() && !body.nodes.empty()) {
        // There is a circular dependency in the nodes, none of them are top-level
        ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_CIRCULAR_DEPENDENCY, ctx_.currentFile, {});
      }

      for (const auto& id : sinkNodes) {
        currentID_.back() = id;
        const auto& ptNode = body.nodes.at(id);
        auto astNode = process(id, ptNode);
        if (!astNode) {
          continue;
        }
        graph_.emplace_back(std::move(astNode));
      }

      currentID_.pop_back();
    }

    return std::move(graph_);
  }

  ast::UniqueNode ScopeBuilder::process(ID id, pt::Node pt) {
    // Remember the ID of the previous node and restore it after processing
    const ID prevID = currentID_.back();
    currentID_.back() = id;
    FLUIR_SCOPE_EXIT { currentID_.back() = prevID; };
    auto astNode = std::visit(*this, pt);
    return astNode;
  }

  ast::Dependency ScopeBuilder::getDependency(ID dependentId, std::optional<unsigned> index) {
    // Find the dependency of ID:index in the tree
    const auto dependencyPt =
      std::ranges::find_if(pt_.conduits, [&dependentId, &index](const pt::Block::Conduits::value_type& v) {
        auto& [_, conduit] = v;
        return std::ranges::any_of(conduit.children, [&dependentId, &index](const pt::Conduit::Output& out) {
          return out.target == dependentId && ((out.index == index) || (!index && out.index == 0));
        });
      });
    if (dependencyPt == pt_.conduits.end()) {
      ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_MISSING_DEPENDENCY, ctx_.currentFile, currentID_);
    }
    const auto& dependencyId = dependencyPt->second.input;
    const auto& dependencyIdx = dependencyPt->second.index;

    // Check that the dependency index isn't already in progress
    if (std::ranges::find(inProgressNodes_, dependencyId) != inProgressNodes_.end()) {
      // We are trying to place a dependency on an in progress node, so
      // there is a circular dependency
      ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_CIRCULAR_DEPENDENCY, ctx_.currentFile, {});
    }
    if (inputs_.contains(dependencyId)) {
      auto parameterID = currentID_;
      parameterID.back() = dependencyId;
      return ast::createDependency<ast::LocalRead>(dependencyId, dependencyIdx, parameterID, FlowGraphLocation{});
    }

    if (promotedIds_.contains(dependencyId)) {
      dependencies_.insert(dependencyId);
      if (index) {
        return ast::createDependencyWithIndex<ast::LocalRead>(
          *index, dependencyId, dependencyIdx, currentID_, FlowGraphLocation{});
      } else {
        return ast::createDependency<ast::LocalRead>(dependencyId, dependencyIdx, currentID_, FlowGraphLocation{});
      }
    }

    auto& pt = pt_.nodes.at(dependencyId);
    // TODO: Handle missing ID
    auto dependency = process(dependencyId, pt);
    // TODO: Handle nonzero index
    alreadyFound_.insert(dependencyId);
    return ast::Dependency{std::move(dependency), index ? std::optional(dependencyIdx) : std::nullopt};
  }

  std::unordered_set<fluir::ID> ScopeBuilder::getIdsPromotedToLocal() const {
    std::unordered_set<fluir::ID> locals;
    std::unordered_map<fluir::ID, size_t> dependents;

    for (const auto& id : pt_.nodes | std::views::keys) {
      dependents.insert({id, 0});
      locals.insert(id);
    }

    for (const auto& conduit : pt_.conduits | std::views::values) {
      // Inputs are already local variables, so ignore conduits originating from them
      if (inputs_.contains(conduit.input)) {
        continue;
      }
      // If the conduit originates from a node, increment its number of dependents
      if (dependents.contains(conduit.input)) {
        dependents.at(conduit.input) += conduit.children.size();
        continue;
      }
      // Report an error if the node isn't found
      auto conduitID = currentID_;
      // TODO: Should conduits have a full ID too?
      conduitID.push_back(conduit.id);
      ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_MISSING_DEPENDENCY, ctx_.currentFile, conduitID);
    }

    // We only keep the nodes with multiple dependents, these are promoted
    // local variables in the AST. The rest will be temporaries
    erase_if(locals, [&](const auto& item) { return dependents.at(item) < 2; });

    return locals;
  }

  std::unordered_set<fluir::ID> ScopeBuilder::getSinkIds() const {
    std::unordered_set<fluir::ID> sinkNodes;

    // Start with all Nodes in the block
    std::ranges::transform(
      pt_.nodes, std::inserter(sinkNodes, sinkNodes.begin()), [](const auto& pair) { return pair.first; });

    // Remove all Nodes that are dependencies of other Nodes
    std::ranges::for_each(pt_.conduits | std::views::values,
                          [&sinkNodes](const auto& conduit) { sinkNodes.erase(conduit.input); });

    return sinkNodes;
  }
}  // namespace fluir::fe
