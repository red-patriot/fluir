#ifndef FLUIR_COMPILER_FRONTEND_ASG_BUILDER_HPP
#define FLUIR_COMPILER_FRONTEND_ASG_BUILDER_HPP

#include <unordered_map>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/asg.hpp"
#include "compiler/utility/context.hpp"

namespace fluir {
  Results<asg::ASG> buildGraph(Context& ctx, const pt::ParseTree& tree);

  Results<asg::DataFlowGraph> buildDataFlowGraph(Context& ctx, pt::Block block);

  class ASGBuilder {
   public:
    static Results<asg::ASG> buildFrom(Context& ctx, const pt::ParseTree& tree);

    fluir::asg::Declaration operator()(const fluir::pt::FunctionDecl& func);

   private:
    Context& ctx_;
    const pt::ParseTree& tree_;
    asg::ASG graph_;

    Results<asg::ASG> run();

    explicit ASGBuilder(Context& ctx, const pt::ParseTree& tree);
  };

  class FlowGraphBuilder {
   public:
    static Results<asg::DataFlowGraph> buildFrom(Context& ctx, pt::Block block);

    asg::Node operator()(const pt::Binary& pt);
    asg::Node operator()(const pt::Unary& pt);
    asg::Node operator()(const pt::Constant& pt);

   private:
    Context& ctx_;
    asg::DataFlowGraph graph_;
    pt::Block block_;
    std::unordered_map<ID, asg::SharedDependency> alreadyFound_;
    std::vector<ID> inProgressNodes_;

    explicit FlowGraphBuilder(Context& ctx, pt::Block block);

    Results<asg::DataFlowGraph> run();

    asg::SharedDependency getDependency(ID dependentId, int index);
  };
}  // namespace fluir

#endif
