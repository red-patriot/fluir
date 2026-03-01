#ifndef FLUIR_COMPILER_FRONTEND_AST_BUILDER_FUNCTION_BUILDER_HPP
#define FLUIR_COMPILER_FRONTEND_AST_BUILDER_FUNCTION_BUILDER_HPP

#include <unordered_map>

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

    ast::UniqueNode operator()(const pt::Binary& pt);
    ast::UniqueNode operator()(const pt::Unary& pt);
    ast::UniqueNode operator()(const pt::Constant& pt);
    ast::UniqueNode operator()(const pt::Call& pt);

   private:
    Context& ctx_;
    ast::DataFlowGraph graph_;
    pt::FunctionDecl pt_;
    FullID currentID_{};
    std::unordered_set<ID> alreadyFound_;
    std::unordered_set<ID> parameters_;
    std::unordered_set<ID> locals_;
    std::vector<ID> inProgressNodes_;
    fluir::dag::NodeSet<ID> dependencies_;

    explicit FunctionAstBuilder(Context& ctx, pt::FunctionDecl pt, std::vector<ID> parents);

    Results<::fluir::ast::FunctionDecl> run();

    ast::UniqueNode process(ID id, pt::Node pt);
    ast::UniqueNode getDependency(ID dependentId, int index);

    std::unordered_set<ID> getLocalNodes();
    std::unordered_set<ID> getSinkNodes() const;
  };

}  // namespace fluir::fe

#endif
