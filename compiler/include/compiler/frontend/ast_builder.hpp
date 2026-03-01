#ifndef FLUIR_COMPILER_FRONTEND_AST_BUILDER_HPP
#define FLUIR_COMPILER_FRONTEND_AST_BUILDER_HPP

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/ast.hpp"
#include "compiler/utility/context.hpp"

namespace fluir {
  Results<ast::AST> buildGraph(Context& ctx, const pt::ParseTree& tree);

  class ASTBuilder {
   public:
    static Results<ast::AST> buildFrom(Context& ctx, const pt::ParseTree& tree);

    Results<ast::Declaration> operator()(const fluir::pt::FunctionDecl& func);

   private:
    Context& ctx_;
    const pt::ParseTree& tree_;
    ast::AST graph_;

    Results<ast::AST> run();

    explicit ASTBuilder(Context& ctx, const pt::ParseTree& tree);
  };
}  // namespace fluir

#endif
