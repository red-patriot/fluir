#include "compiler/frontend/ast_builder.hpp"

#include <algorithm>
#include <ranges>
#include <variant>

#include "compiler/frontend/ast_builder/function_builder.hpp"

namespace fluir {
  Results<ast::AST> buildGraph(Context& ctx, const pt::ParseTree& tree) { return ASTBuilder::buildFrom(ctx, tree); }

  Results<ast::AST> ASTBuilder::buildFrom(Context& ctx, const pt::ParseTree& tree) {
    ASTBuilder builder{ctx, tree};
    return builder.run();
  }

  ASTBuilder::ASTBuilder(Context& ctx, const pt::ParseTree& tree) : ctx_(ctx), tree_(tree) { }

  Results<ast::Declaration> ASTBuilder::operator()(const fluir::pt::FunctionDecl& func) {
    auto funcAst = fe::FunctionAstBuilder::buildAst(ctx_, func);

    if (!funcAst) {
      return NoResult;
    }
    fluir::ast::FunctionDecl decl = std::move(*funcAst);

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
}  // namespace fluir
