#ifndef FLUIR_COMPILER_UTILITY_TOPOLOGICAL_SORT_HPP
#define FLUIR_COMPILER_UTILITY_TOPOLOGICAL_SORT_HPP

#include <algorithm>
#include <deque>
#include <unordered_map>
#include <vector>

namespace fluir {
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
}  // namespace fluir

#endif
