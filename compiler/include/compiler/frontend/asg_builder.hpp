#ifndef FLUIR_COMPILER_FRONTEND_ASG_BUILDER_HPP
#define FLUIR_COMPILER_FRONTEND_ASG_BUILDER_HPP

#include <unordered_map>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/asg.hpp"
#include "compiler/utility/results.hpp"

namespace fluir {
  Results<asg::ASG> buildGraph(const pt::ParseTree& tree);

  Results<asg::DataFlowGraph> buildDataFlowGraph(pt::Block block);

  class ASGBuilder {
   public:
    static Results<asg::ASG> buildFrom(const pt::ParseTree& tree);

    fluir::asg::Declaration operator()(const fluir::pt::FunctionDecl& func);

   private:
    const pt::ParseTree& tree_;
    asg::ASG graph_;
    Diagnostics diagnostics_;

    Results<asg::ASG> run();

    explicit ASGBuilder(const pt::ParseTree& tree);
  };

  class FlowGraphBuilder {
   public:
    static Results<asg::DataFlowGraph> buildFrom(pt::Block block);

    asg::Node operator()(const pt::Binary& pt);
    asg::Node operator()(const pt::Unary& pt);
    asg::Node operator()(const pt::Constant& pt);

   private:
    asg::DataFlowGraph graph_;
    pt::Block block_;
    Diagnostics diagnostics_;
    std::unordered_map<ID, asg::SharedDependency> alreadyFound_;
    std::vector<ID> inProgressNodes_;

    explicit FlowGraphBuilder(pt::Block block);

    Results<asg::DataFlowGraph> run();

    asg::SharedDependency getDependency(ID id);
  };
}  // namespace fluir

#endif
