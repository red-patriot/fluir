#include "compiler/frontend/ast_builder/function_builder.hpp"

#include <algorithm>
#include <ranges>
#include <unordered_set>

#include "compiler/frontend/ast_builder/scope_builder.hpp"

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

  Results<ast::FunctionDecl> FunctionAstBuilder::run() {
    std::vector<ID> outputs;
    if (pt_.output && pt_.output->ret) {
      outputs.emplace_back(pt_.output->ret->id);
    }

    std::optional<ast::FunctionDecl::Return> returnVal{std::nullopt};
    if (pt_.output && pt_.output->ret) {
      returnVal.emplace();
      returnVal->id = pt_.output->ret->id;
      returnVal->typeName = pt_.output->ret->typeName;
      // TODO: Support multiple return values
    }

    std::vector<ast::FunctionDecl::Parameter> parameters;
    if (pt_.input) {
      std::ranges::transform(pt_.input->parameters, std::inserter(parameters, parameters.begin()), [](const auto& p) {
        return ast::FunctionDecl::Parameter{p.id, p.name, p.typeName};
      });
    }

    auto graph = ScopeBuilder::buildAst(ctx_, pt_.body, parameters_, outputs, currentID_);
    if (!graph) {
      return std::nullopt;
    }

    // TODO: Store some type information for type checking?
    // TODO: Update functions to use bundle/unbundle for multiple return values
    if (pt_.output.has_value()) {
      // HACK: Current behavior for function decls is only 0 or 1 return and it is written to a local slot,
      //  Wrap the last node in a LocalWrite so that stays for now
      auto outputNode = std::move(graph->back());
      auto outputId = outputNode->fullId();
      outputId.back() = outputs.back();  // Replace the internal ID of the node to write to the return value
      auto write = ast::createNode<ast::LocalWrite>(
        std::move(outputId), ast::Dependency(std::move(outputNode)), fluir::FlowGraphLocation{});
      graph->back() = std::move(write);
    }

    ast::FunctionDecl ast{.id = pt_.id,
                          .location = pt_.location,
                          .name = pt_.name,
                          .statements = std::move(*graph),
                          .parameters = parameters,
                          .returnValue = returnVal};
    return ast;
  }
}  // namespace fluir::fe
