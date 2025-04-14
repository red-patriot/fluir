#ifndef FLUIR_COMPILER_AFG_FLOW_GRAPH_HPP
#define FLUIR_COMPILER_AFG_FLOW_GRAPH_HPP

#include <vector>

#include "fluir/compiler/afg/node.hpp"

namespace fluir::afg {
  class FlowGraph {
   public:
    void add(UniqueNode node);

    Node& at(size_t index);
    const Node& at(size_t index) const;

   private:
    std::vector<UniqueNode> nodes_{};
  };
}  // namespace fluir::afg

#endif
