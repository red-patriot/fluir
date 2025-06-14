module;
#include <unordered_map>
#include <vector>

export module fluir.frontend.asg_builder;

import fluir.models.asg;
import fluir.frontend.parse_tree;
import fluir.utility.results;
import fluir.utility.context;

namespace fluir {
  export Results<asg::ASG> buildGraph(Context& ctx, const pt::ParseTree& tree);

  export Results<asg::DataFlowGraph> buildDataFlowGraph(Context& ctx, pt::Block block);

  export class ASGBuilder {
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

  export class FlowGraphBuilder {
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
