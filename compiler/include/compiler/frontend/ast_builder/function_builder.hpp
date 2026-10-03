#ifndef FLUIR_COMPILER_FRONTEND_AST_BUILDER_FUNCTION_BUILDER_HPP
#define FLUIR_COMPILER_FRONTEND_AST_BUILDER_FUNCTION_BUILDER_HPP

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/ast.hpp"
#include "compiler/utility/context.hpp"
#include "compiler/utility/topological_sort.hpp"

namespace fluir::fe {
  class FunctionAstBuilder {
   public:
    static Results<ast::FunctionDecl> buildAst(Context& ctx,
                                               const pt::FunctionDecl& functionDecl,
                                               std::vector<ID> parents = {});

   private:
    Context& ctx_;
    pt::FunctionDecl pt_;
    FullID currentID_{};
    std::unordered_set<ID> parameters_;

    explicit FunctionAstBuilder(Context& ctx, pt::FunctionDecl pt, std::vector<ID> parents);

    Results<::fluir::ast::FunctionDecl> run();
  };

}  // namespace fluir::fe

#endif
