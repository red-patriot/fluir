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
    std::unordered_set<ID> outputs;
    if (pt_.output && pt_.output->ret) {
      outputs.insert(pt_.output->ret->id);
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

    // TODO: Store some type information for type checking

    ast::FunctionDecl ast{.id = pt_.id,
                          .location = pt_.location,
                          .name = pt_.name,
                          .statements = std::move(*graph),
                          .parameters = parameters,
                          .returnValue = returnVal};
    return ast;
  }
}  // namespace fluir::fe
