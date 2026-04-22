#include "compiler/frontend/ast_builder/function_builder.hpp"

#include <algorithm>
#include <ranges>
#include <unordered_set>
#include <variant>

#include "compiler/utility/scope_guard.hpp"

namespace fluir::fe {
  Results<ast::FunctionDecl> FunctionAstBuilder::buildAst(Context& ctx,
                                                          const pt::FunctionDecl& functionDecl,
                                                          std::vector<ID> parents) {
    FunctionAstBuilder builder{ctx, functionDecl, std::move(parents)};
    return builder.run();
  }

  FunctionAstBuilder::FunctionAstBuilder(Context& ctx, pt::FunctionDecl pt, std::vector<ID> parents) :
    ctx_(ctx), pt_(std::move(pt)), currentID_(std::move(parents)) {
    currentID_.push_back(pt_.id);
    if (pt_.input) {
      std::ranges::transform(
        pt_.input->parameters, std::inserter(parameters_, parameters_.begin()), [](const auto& p) { return p.id; });
    }
  }

  ast::UniqueNode FunctionAstBuilder::operator()(const pt::Binary& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };
    return std::make_unique<ast::BinaryOp>(
      pt.op, getDependency(pt.id, 0), getDependency(pt.id, 1), currentID_, pt.location);
  }

  ast::UniqueNode FunctionAstBuilder::operator()(const pt::Unary& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };
    return std::make_unique<ast::UnaryOp>(pt.op, getDependency(pt.id, 0), currentID_, pt.location);
  };

  ast::UniqueNode FunctionAstBuilder::operator()(const pt::Constant& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };
    return std::make_unique<ast::Constant>(pt.value, currentID_, pt.location);
  }

  ast::UniqueNode FunctionAstBuilder::operator()(const pt::Call& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };

    auto ptArgs = pt.arguments;
    std::ranges::sort(
      ptArgs, [](const pt::Call::Argument& lhs, const pt::Call::Argument& rhs) { return lhs.index < rhs.index; });

    std::vector<ast::UniqueNode> astArgs;
    for (const auto& [name, index] : ptArgs) {
      auto argument = getDependency(pt.id, index);
      astArgs.emplace_back(std::move(argument));
    }

    return ast::createDependency<ast::Call>(pt.target, std::move(astArgs), currentID_, pt.location);
  }

  Results<ast::FunctionDecl> FunctionAstBuilder::run() {
    const auto& body = pt_.body;
    alreadyFound_.reserve(body.nodes.size());
    locals_ = getLocalNodes();
    currentID_.push_back(INVALID_ID);  // Make a space for the next level of IDs
    dag::Arcs<ID> dependencies;
    for (const auto& local : locals_) {
      currentID_.back() = local;
      auto astNode = process(local, body.nodes.at(local));
      alreadyFound_.insert(local);
      auto write = ast::createDependency<ast::LocalWrite>(std::move(astNode), astNode->location());
      dependencies.insert({local, std::move(dependencies_)});
      dependencies_ = {};
      graph_.emplace_back(std::move(write));
    }

    // Topological sort of graph_ so locals are written/read in the right order
    if (!dag::topologicalSort(graph_, dependencies, [](const ast::UniqueNode& node) -> ID { return node->id(); })) {
      ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_CIRCULAR_DEPENDENCY, ctx_.currentFile, {});
    }

    std::optional<ast::FunctionDecl::Return> returnVal{std::nullopt};
    if (pt_.output && pt_.output->ret) {
      returnVal.emplace();
      returnVal->id = pt_.output->ret->id;
      returnVal->typeName = pt_.output->ret->typeName;
      FullID fullID = currentID_;
      fullID.push_back(returnVal->id);
      // TODO: Support multiple return values
      auto returnedNode = getDependency(pt_.output->ret->id, 0);
      auto write =
        ast::createDependency<ast::LocalWrite>(std::move(fullID), std::move(returnedNode), fluir::FlowGraphLocation{});
      graph_.emplace_back(std::move(write));
    }

    // If there are more pending nodes, process them now
    if (alreadyFound_.size() != body.nodes.size()) {
      // Find a Node without dependents in the graph
      const auto sinkNodes = getSinkNodes();
      if (sinkNodes.empty() && !body.nodes.empty()) {
        // There is a circular dependency in the nodes, none of them are top-level
        ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_CIRCULAR_DEPENDENCY, ctx_.currentFile, {});
      }

      for (const auto& id : sinkNodes) {
        currentID_.back() = id;
        const auto& ptNode = body.nodes.at(id);
        auto astNode = process(id, ptNode);
        graph_.emplace_back(std::move(astNode));
      }

      currentID_.pop_back();
    }

    std::vector<ast::FunctionDecl::Parameter> parameters;
    if (pt_.input) {
      std::ranges::transform(pt_.input->parameters, std::inserter(parameters, parameters.begin()), [](const auto& p) {
        return ast::FunctionDecl::Parameter{p.id, p.name, p.typeName};
      });
    }

    // TODO: Store some type information for type checking
    ast::FunctionDecl ast{.id = pt_.id,
                          .location = pt_.location,
                          .name = pt_.name,
                          .statements = std::move(graph_),
                          .parameters = parameters,
                          .returnValue = returnVal};
    return ast;
  }

  ast::UniqueNode FunctionAstBuilder::process(ID id, pt::Node pt) {
    // Remember the ID of the previous node and restore it after processing
    const ID prevID = currentID_.back();
    currentID_.back() = id;
    FLUIR_SCOPE_EXIT { currentID_.back() = prevID; };
    auto astNode = std::visit(*this, pt);
    return astNode;
  }

  ast::UniqueNode FunctionAstBuilder::getDependency(ID dependentId, int index) {
    // Find the dependency of ID:index in the tree
    const auto dependencyPt =
      std::ranges::find_if(pt_.body.conduits, [&dependentId, &index](const pt::Block::Conduits::value_type& v) {
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
      ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_CIRCULAR_DEPENDENCY, ctx_.currentFile, {});
    }
    if (parameters_.contains(dependencyId)) {
      auto parameterID = currentID_;
      parameterID.back() = dependencyId;
      return ast::createDependency<ast::LocalRead>(dependencyId, parameterID, FlowGraphLocation{});
    }

    if (locals_.contains(dependencyId)) {
      dependencies_.insert(dependencyId);
      return ast::createDependency<ast::LocalRead>(dependencyId, currentID_, FlowGraphLocation{});
    }

    auto& pt = pt_.body.nodes.at(dependencyId);  // TODO: Handle missing ID
    auto dependency = process(dependencyId, pt);
    alreadyFound_.insert(dependencyId);
    return dependency;
  }

  std::unordered_set<fluir::ID> FunctionAstBuilder::getLocalNodes() {
    const auto& block = pt_.body;
    std::unordered_set<fluir::ID> locals;
    std::unordered_map<fluir::ID, size_t> dependents;

    for (const auto& id : block.nodes | std::views::keys) {
      dependents.insert({id, 0});
      locals.insert(id);
    }

    for (const auto& conduit : block.conduits | std::views::values) {
      // If the conduit originates from a parameter, no work needs to be done
      if (parameters_.contains(conduit.input)) {
        continue;
      }
      // If the conduit originates from a local node, increment its number of dependents
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

    // We only keep the nodes with multiple dependents, these will become
    // local variables in the AST. The rest will be temporaries
    erase_if(locals, [&](const auto& item) { return dependents.at(item) < 2; });

    return locals;
  }

  std::unordered_set<fluir::ID> FunctionAstBuilder::getSinkNodes() const {
    std::unordered_set<fluir::ID> sinkNodes;
    if (pt_.output && pt_.output->ret) {
      // The return value is a sink if it exists
      sinkNodes.insert(pt_.output->ret->id);
    }
    const auto& block = pt_.body;

    // Start with all Nodes in the block
    std::ranges::transform(
      block.nodes, std::inserter(sinkNodes, sinkNodes.begin()), [](const auto& pair) { return pair.first; });

    // Remove all Nodes that are dependencies of other Nodes
    std::ranges::for_each(block.conduits | std::views::values,
                          [&sinkNodes](const auto& conduit) { sinkNodes.erase(conduit.input); });

    return sinkNodes;
  }
}  // namespace fluir::fe
