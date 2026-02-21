#include "compiler/frontend/ast_builder/function_builder.hpp"

#include <algorithm>
#include <ranges>
#include <unordered_set>
#include <variant>

#include "compiler/utility/scope_guard.hpp"

namespace fluir::fe {
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

    std::unordered_set<fluir::ID> getLocalNodes(Context& ctx, const fluir::pt::Block& block, FullID parentID) {
      std::unordered_set<fluir::ID> locals;
      std::unordered_map<fluir::ID, size_t> dependents;

      for (const auto& id : block.nodes | std::views::keys) {
        dependents.insert({id, 0});
        locals.insert(id);
      }

      for (const auto& conduit : block.conduits | std::views::values) {
        if (!dependents.contains(conduit.input)) {
          auto conduitID = parentID;
          // TODO: Should conduits have a full ID too?
          conduitID.push_back(conduit.id);
          ctx.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_MISSING_DEPENDENCY, ctx.currentFile, conduitID);
        }
        dependents.at(conduit.input) += conduit.children.size();
      }

      // We only keep the nodes with multiple dependents, these will become
      // local variables in the AST. The rest will be temporaries
      erase_if(locals, [&](const auto& item) { return dependents.at(item) < 2; });

      return locals;
    }
  }  // namespace

  Results<ast::FunctionDecl> FunctionAstBuilder::buildAst(Context& ctx,
                                                          const pt::FunctionDecl& functionDecl,
                                                          std::vector<ID> parents) {
    FunctionAstBuilder builder{ctx, functionDecl, std::move(parents)};
    return builder.run();
  }

  FunctionAstBuilder::FunctionAstBuilder(Context& ctx, pt::FunctionDecl pt, std::vector<ID> parents) :
    ctx_(ctx), pt_(std::move(pt)), currentID_(std::move(parents)) { }

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

  ast::UniqueNode FunctionAstBuilder::operator()(const pt::Call&) { assert(false && "NOT IMPLEMENTED!"); }

  Results<ast::FunctionDecl> FunctionAstBuilder::run() {
    const auto& body = pt_.body;
    alreadyFound_.reserve(body.nodes.size());
    locals_ = getLocalNodes(ctx_, body, currentID_);
    currentID_.push_back(INVALID_ID);  // Make a space for the next level of IDs
    dag::Arcs<ID> dependencies;
    for (const auto& local : locals_) {
      currentID_.back() = local;
      auto astNode = process(local, body.nodes.at(local));
      auto write = ast::createDependency<ast::LocalWrite>(std::move(astNode), astNode->location());
      dependencies.insert({local, std::move(dependencies_)});
      dependencies_ = {};
      graph_.emplace_back(std::move(write));
    }

    // Topological sort of graph_ so locals are written/read in the right order
    if (!dag::topologicalSort(graph_, dependencies, [](const ast::UniqueNode& node) -> ID { return node->id(); })) {
      ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_CIRCULAR_DEPENDENCY, ctx_.currentFile, {});
    }

    // Find a Node without dependents in the graph
    const auto sinkNodes = getSinkNodes(body);
    if (sinkNodes.empty() && !body.nodes.empty()) {
      // There is a circular dependency in the nodes, none of them are top-level
      ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_CIRCULAR_DEPENDENCY, ctx_.currentFile, {});
    }

    for (const auto& ptNode : sinkNodes) {
      currentID_.back() = ptNode;
      auto astNode = process(ptNode, body.nodes.at(ptNode));
      graph_.emplace_back(std::move(astNode));
    }

    currentID_.pop_back();
    ast::FunctionDecl ast{.id = pt_.id, .location = pt_.location, .name = pt_.name, .statements = std::move(graph_)};
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

    if (locals_.contains(dependencyId)) {
      dependencies_.insert(dependencyId);
      return ast::createDependency<ast::LocalRead>(dependencyId, currentID_, FlowGraphLocation{});
    }

    auto& pt = pt_.body.nodes.at(dependencyId);  // TODO: Handle missing ID
    auto dependency = process(dependencyId, pt);
    alreadyFound_.insert(dependencyId);
    return dependency;
  }
}  // namespace fluir::fe
