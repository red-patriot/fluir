#ifndef FLUIR_COMPILER_UTILITY_TOPOLOGICAL_SORT_HPP
#define FLUIR_COMPILER_UTILITY_TOPOLOGICAL_SORT_HPP

#include <algorithm>
#include <deque>
#include <unordered_map>
#include <vector>

namespace fluir::dag {
  template <typename T>
  using Nodes = std::vector<T>;

  template <typename T>
  using Arcs = std::unordered_map<T, Nodes<T>>;

  /** Sorts the directed graph topologically, if possible.
   *
   * \param nodes The Nodes in the graph
   * \param arcs A map from each node to all the nodes that depend on it.
   * \note It is assumed that the set of values in nodes and the set of
   *       values in keys and values in arcs must be equal.
   * \note Each node is assumed to have a unique value
   */
  template <typename Value>
  bool topologicalSort(Nodes<Value>& nodes, const Arcs<Value>& arcs) {
    // Use Kahn's Algorithm to do a topological sort
    std::unordered_map<Value, size_t> inDegree;
    std::unordered_map<Value, std::vector<Value>> dependentArcs;

    // Initialize in-degrees to 0
    std::ranges::for_each(nodes, [&](const Value& node) { inDegree.insert({node, 0}); });

    // Calculate in-degrees and reverse dependencies
    for (const auto& [source, targets] : arcs) {
      for (const auto& target : targets) {
        ++inDegree[target];
        dependentArcs[source].push_back(target);
      }
    }

    // Find all nodes with in-degree 0
    std::deque<Value> ready;
    for (const auto& node : nodes) {
      if (inDegree[node] == 0) {
        ready.push_back(node);
      }
    }

    // Process nodes in topological order
    auto b = nodes.begin();

    while (!ready.empty()) {
      Value current = ready.front();
      ready.pop_front();
      *b++ = current;

      // Reduce in-degree of dependent nodes
      if (dependentArcs.count(current)) {
        for (const auto& dependent : dependentArcs[current]) {
          if (--inDegree[dependent] == 0) {
            ready.push_back(dependent);
          }
        }
      }
    }

    if (b != nodes.end()) {
      return false;
    }

    return true;
  }

  template <typename Value, typename IdFunc>
  bool topologicalSort(Nodes<Value>& nodes,
                       const Arcs<std::invoke_result_t<IdFunc, const Value&>>& arcs,
                       IdFunc getId) {
    using Id = std::invoke_result_t<IdFunc, const Value&>;

    // Build a mapping of the IDs to the original nodes
    std::unordered_map<Id, size_t> idToIndex;
    Nodes<Id> ids;
    ids.reserve(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
      ids.push_back(getId(nodes[i]));
      idToIndex[getId(nodes[i])] = i;
    }

    if (!topologicalSort(ids, arcs)) {
      return false;
    }

    // Reorder nodes according to sorted indices
    Nodes<Value> sorted;
    sorted.reserve(nodes.size());
    for (const auto& id : ids) {
      auto idx = idToIndex.at(id);
      sorted.push_back(std::move(nodes[idx]));
    }
    nodes = std::move(sorted);

    return true;
  }
}  // namespace fluir::dag

#endif
