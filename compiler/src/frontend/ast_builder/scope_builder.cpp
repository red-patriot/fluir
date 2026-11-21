#include "compiler/frontend/ast_builder/scope_builder.hpp"

#include <algorithm>
#include <ranges>
#include <unordered_set>
#include <variant>

#include "compiler/frontend/text_parse.hpp"
#include "compiler/utility/scope_guard.hpp"

namespace fluir::fe {
  Results<ast::DataFlowGraph> ScopeBuilder::buildAst(Context& ctx,
                                                     const pt::Block& block,
                                                     const std::unordered_set<ID>& inputs,
                                                     const std::vector<ID>& outputs,
                                                     FullID id) {
    ScopeBuilder builder{ctx, block, inputs, outputs, std::move(id)};
    return builder.run();
  }

  Results<ast::SubGraph> ScopeBuilder::buildSubGraph(Context& ctx,
                                                     const pt::Block& block,
                                                     const std::unordered_set<ID>& inputs,
                                                     const std::vector<ID>& outputs,
                                                     FullID id) {
    ScopeBuilder builder{ctx, block, inputs, outputs, std::move(id)};
    auto raw = builder.run();
    if (!raw) {
      return NoResult;
    }
    if (outputs.empty()) {
      return ast::SubGraph{.sinks = std::move(*raw), .output = nullptr};
    }
    auto output = std::move(raw->back());
    raw->pop_back();
    return ast::SubGraph{.sinks = std::move(*raw), .output = std::move(output)};
  }

  ScopeBuilder::ScopeBuilder(
    Context& ctx, pt::Block pt, const std::unordered_set<ID>& inputs, const std::vector<ID>& outputs, FullID id) :
    ctx_(ctx), pt_(std::move(pt)), currentID_(std::move(id)), inputs_(inputs), outputs_(outputs) { }

  ast::UniqueNode ScopeBuilder::operator()(const pt::Binary& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };
    auto lhs = getDependency(pt.id, 0);
    auto rhs = getDependency(pt.id, 1);
    if (!lhs || !rhs) {
      return nullptr;
    }
    return std::make_unique<ast::BinaryOp>(pt.op, std::move(lhs), std::move(rhs), currentID_, pt.location);
  }

  ast::UniqueNode ScopeBuilder::operator()(const pt::Unary& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };
    auto operand = getDependency(pt.id, 0);
    if (!operand) {
      return nullptr;
    }
    return std::make_unique<ast::UnaryOp>(pt.op, std::move(operand), currentID_, pt.location);
  };

  template <typename T>
  bool ScopeBuilder::handleNumericParseError(const std::expected<T, NumberParseError>& result, const pt::Constant& pt) {
    if (result.has_value()) {
      return true;
    }
    switch (result.error()) {
      case NumberParseError::SUCCESS:
        return true;
      case NumberParseError::CANNOT_PARSE_NUMBER:
        ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
                                          ctx_.currentFile,
                                          currentID_,
                                          "Expected a number while parsing <{}>, got '{}'",
                                          toString(pt.typeTag),
                                          pt.value);
        break;
      case NumberParseError::RESULT_OUT_OF_BOUNDS:
        ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_NUMBER_OUT_OF_RANGE,
                                          ctx_.currentFile,
                                          currentID_,
                                          "Number '{}' is out of range for type {}",
                                          pt.value,
                                          toString(pt.typeTag));
        break;
    }
    return false;
  }

  ast::UniqueNode ScopeBuilder::operator()(const pt::Constant& pt) {
    using namespace literals_types;
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };
    switch (pt.typeTag) {
        // TODO: Handle error
      case pt::Constant::F64:
        {
          auto result = parseDouble(pt.value);
          handleNumericParseError(result, pt);
          return std::make_unique<ast::Constant>(result.value(), currentID_, pt.location);
        }
      case pt::Constant::I8:
        {
          auto val = parseNumber<I8>(pt.value);
          handleNumericParseError(val, pt);

          return std::make_unique<ast::Constant>(val.value(), currentID_, pt.location);
        }
      case pt::Constant::I16:
        {
          auto val = parseNumber<I16>(pt.value);
          handleNumericParseError(val, pt);

          return std::make_unique<ast::Constant>(val.value(), currentID_, pt.location);
        }
      case pt::Constant::I32:
        {
          auto val = parseNumber<I32>(pt.value);
          handleNumericParseError(val, pt);
          return std::make_unique<ast::Constant>(val.value(), currentID_, pt.location);
        }
      case pt::Constant::I64:
        {
          auto val = parseNumber<I64>(pt.value);
          handleNumericParseError(val, pt);
          return std::make_unique<ast::Constant>(val.value(), currentID_, pt.location);
        }
      case pt::Constant::U8:
        {
          auto val = parseNumber<U8>(pt.value);
          handleNumericParseError(val, pt);
          return std::make_unique<ast::Constant>(val.value(), currentID_, pt.location);
        }
      case pt::Constant::U16:
        {
          auto val = parseNumber<U16>(pt.value);
          handleNumericParseError(val, pt);
          return std::make_unique<ast::Constant>(val.value(), currentID_, pt.location);
        }
      case pt::Constant::U32:
        {
          auto val = parseNumber<U32>(pt.value);
          handleNumericParseError(val, pt);
          return std::make_unique<ast::Constant>(val.value(), currentID_, pt.location);
        }
      case pt::Constant::U64:
        {
          auto val = parseNumber<U64>(pt.value);
          handleNumericParseError(val, pt);
          return std::make_unique<ast::Constant>(val.value(), currentID_, pt.location);
        }
      case pt::Constant::BOOL:
        {
          if (pt.value != "true" && pt.value != "false") {
            ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
                                              ctx_.currentFile,
                                              currentID_,
                                              "Expected 'true' or 'false' while parsing <{}>, got '{}'",
                                              toString(pt.typeTag),
                                              pt.value);
            return nullptr;
          }
          return std::make_unique<ast::Constant>(pt.value == "true", currentID_, pt.location);
        }
    }
    return nullptr;
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
    if (std::ranges::any_of(astArgs, [](const auto& arg) { return !arg; })) {
      return nullptr;
    }

    return ast::createNode<ast::Call>(pt.target, std::move(astArgs), currentID_, pt.location);
  }

  ast::UniqueNode ScopeBuilder::operator()(const pt::Conditional& pt) {
    inProgressNodes_.emplace_back(pt.id);
    FLUIR_SCOPE_EXIT { inProgressNodes_.pop_back(); };

    auto condition = getDependency(pt.id, ast::Conditional::CONDITION_INDEX);
    std::unordered_set<ID> inputIds = {pt.condition.innerId};
    std::vector<ast::ScopeInput> inputs;
    for (const auto [idx, ptInput] : pt.inputs | std::ranges::views::enumerate) {
      inputs.emplace_back(
        getDependency(pt.id,
                      static_cast<unsigned>(idx + 1)),  // Index 0 is the condition, so inc by 1 for the other inputs
        ptInput.innerId);
      inputIds.insert(ptInput.innerId);
    }
    if (std::ranges::any_of(inputs, [](const auto& in) { return !in.node; })) {
      return nullptr;
    }

    std::vector<ID> outputIds;
    std::ranges::transform(pt.outputs, std::back_inserter(outputIds), [](const auto& o) { return o.innerId; });

    ast::SubGraph thenBody;
    {
      currentID_.emplace_back(ast::Conditional::THEN_BODY);
      FLUIR_SCOPE_EXIT { currentID_.pop_back(); };
      auto maybeThen = buildSubGraph(ctx_, *pt.thenScope, inputIds, outputIds, currentID_);
      if (maybeThen) {
        thenBody = std::move(*maybeThen);
      }
    }
    ast::SubGraph elseBody;
    {
      currentID_.emplace_back(ast::Conditional::ELSE_BODY);
      FLUIR_SCOPE_EXIT { currentID_.pop_back(); };
      auto maybeElse = buildSubGraph(ctx_, *pt.elseScope, inputIds, outputIds, currentID_);
      if (maybeElse) {
        elseBody = std::move(*maybeElse);
      }
    }

    std::vector<ID> outputs;
    std::ranges::transform(pt.outputs, std::back_inserter(outputs), [](const auto& o) { return o.innerId; });
    if (outputs.size() > 1) {
      bundledIds_.insert(pt.id);
    }

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
    bool failed = false;
    for (const auto& local : promotedIds_) {
      currentID_.back() = local;
      auto astNode = process(local, body.nodes.at(local));
      if (!astNode) {
        failed = true;
        continue;
      }
      alreadyFound_.insert(local);
      const auto location = astNode->location();
      auto write = ast::createNode<ast::LocalWrite>(ast::createDependency(std::nullopt, std::move(astNode)), location);
      dependencies.insert({local, std::move(dependencies_)});
      dependencies_ = {};
      graph_.emplace_back(std::move(write));
    }
    if (failed) {
      return Results<ast::DataFlowGraph>{};
    }

    // Topological sort of graph_ so locals are written/read in the right order
    if (!dag::topologicalSort(graph_, dependencies, [](const ast::UniqueNode& node) -> ID { return node->id(); })) {
      ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_CIRCULAR_DEPENDENCY, ctx_.currentFile, {});
    }

    // Process outputs
    auto outputs = processOutputs(outputs_, currentID_, FlowGraphLocation{});

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
          failed = true;
          continue;
        }
        graph_.emplace_back(std::move(astNode));
      }

      currentID_.pop_back();
      if (failed) {
        return Results<ast::DataFlowGraph>{};
      }
    }

    if (outputs) {
      graph_.emplace_back(std::move(*outputs));
    }
    return std::move(graph_);
  }

  ast::UniqueNode ScopeBuilder::process(ID id, pt::Node pt) {
    try {
      // Remember the ID of the previous node and restore it after processing
      const ID prevID = currentID_.back();
      currentID_.back() = id;
      FLUIR_SCOPE_EXIT { currentID_.back() = prevID; };
      auto astNode = std::visit(*this, pt);
      return astNode;
    } catch (const diagnostic::Panic&) {
      // Allow returning multiple errors
      return nullptr;
    }
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
      return ast::createDependency<ast::LocalRead>(dependencyId, parameterID, FlowGraphLocation{});
    }

    if (promotedIds_.contains(dependencyId)) {
      dependencies_.insert(dependencyId);
      if (bundledIds_.contains(dependencyId)) {
        auto read = ast::createDependency<ast::LocalRead>(dependencyId, currentID_, FlowGraphLocation{});
        return ast::createDependency<ast::Unbundle>(
          currentID_, FlowGraphLocation{}, std::move(read), static_cast<size_t>(dependencyIdx));
      } else {
        return ast::createDependency<ast::LocalRead>(dependencyId, currentID_, FlowGraphLocation{});
      }
    }

    auto& pt = pt_.nodes.at(dependencyId);
    // TODO: Handle missing ID
    auto dependency = process(dependencyId, pt);
    if (!dependency) {
      return ast::Dependency{nullptr};
    }
    // TODO: Handle nonzero index
    alreadyFound_.insert(dependencyId);
    if (bundledIds_.contains(dependencyId)) {
      dependency = ast::createNode<ast::Unbundle>(
        currentID_, FlowGraphLocation{}, ast::Dependency{std::move(dependency)}, static_cast<size_t>(dependencyIdx));
    } else {
      if (dependencyIdx != 0) {
        ctx_.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_MISSING_DEPENDENCY,
                                          ctx_.currentFile,
                                          currentID_,
                                          "Node depends on nonexistent output index {}, only 0 allowed.",
                                          dependencyIdx);
      }
    }
    return ast::Dependency{std::move(dependency)};
  }

  std::optional<ast::UniqueNode> ScopeBuilder::processOutputs(const std::vector<ID>& outputIDs,
                                                              FullID id,
                                                              FlowGraphLocation location) {
    if (outputIDs.empty()) {
      return std::nullopt;
    }
    id.pop_back();  // Pop the temporary ID off, to just have the parent ID here

    std::vector<ast::Dependency> outputs;
    for (const auto& output : outputIDs) {
      FullID outputID = currentID_;
      outputID.push_back(output);
      auto dependency = getDependency(output, std::nullopt);
      outputs.emplace_back(std::move(dependency));
    }

    if (outputIDs.size() > 1) {
      return ast::createNode<ast::Bundle>(std::move(id), location, std::move(outputs));
    } else {
      return std::move(outputs.front().child());
    }
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
