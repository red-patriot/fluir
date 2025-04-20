#ifndef FLUIR_COMPILER_FRONTEND_AST_BUILDER_HPP
#define FLUIR_COMPILER_FRONTEND_AST_BUILDER_HPP

#include "compiler/frontend/ast/ast.hpp"
#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/utility/results.hpp"

namespace fluir {
  Results<ast::ASG> buildGraph(const pt::ParseTree& tree);
  //   Results<ast::AST> buildTree(const ast::ASG& graph);

  Results<ast::DataFlowGraph> buildDataFlowGraph(pt::Block block);

  class ASGBuilder {
   public:
    static Results<ast::ASG> buildFrom(const pt::ParseTree& tree);

    fluir::ast::Declaration operator()(const fluir::pt::FunctionDecl& func);

   private:
    ast::ASG graph_;
    Diagnostics diagnostics_;

    ASGBuilder() = default;
  };

  class FlowGraphBuilder {
   public:
    static Results<ast::DataFlowGraph> buildFrom(pt::Block block);

    ast::Node operator()(const pt::Binary& pt);
    ast::Node operator()(const pt::Unary& pt);
    ast::Node operator()(const pt::Constant& pt);

   private:
    ast::DataFlowGraph graph_;
    pt::Block block_;
    Diagnostics diagnostics_;
    std::unordered_map<ID, ast::SharedDependency> alreadyFound_;

    FlowGraphBuilder(pt::Block block);

    Results<ast::DataFlowGraph> run();

    ast::SharedDependency getDependency(ID id);
  };
}  // namespace fluir

#endif
