#ifndef FLUIR_COMPILER_FRONTEND_AST_BUILDER_HPP
#define FLUIR_COMPILER_FRONTEND_AST_BUILDER_HPP

#include <unordered_map>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/ast.hpp"
#include "compiler/utility/context.hpp"
#include "compiler/utility/topological_sort.hpp"

namespace fluir {
  Results<ast::AST> buildGraph(Context& ctx, const pt::ParseTree& tree);

  Results<ast::DataFlowGraph> buildDataFlowGraph(Context& ctx, pt::Block block, std::vector<ID> parents = {});

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

  class FlowGraphBuilder {
   public:
    static Results<ast::DataFlowGraph> buildFrom(Context& ctx, pt::Block block, std::vector<ID> parents);

    ast::UniqueNode operator()(const pt::Binary& pt);
    ast::UniqueNode operator()(const pt::Unary& pt);
    ast::UniqueNode operator()(const pt::Constant& pt);

   private:
    Context& ctx_;
    ast::DataFlowGraph graph_;
    pt::Block block_;
    ID current_{INVALID_ID};
    std::unordered_set<ID> alreadyFound_;
    std::unordered_set<ID> locals_;
    std::vector<ID> inProgressNodes_;
    fluir::dag::NodeSet<ID> dependencies_;
    std::vector<ID> parents_;

    explicit FlowGraphBuilder(Context& ctx, pt::Block block, std::vector<ID> parents);

    Results<ast::DataFlowGraph> run();

    ast::SharedDependency getDependency(ID dependentId, int index);
    ast::SharedDependency createLocal();
  };
}  // namespace fluir

#endif
