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
   * @tparam Value the value type of each node
   * @param nodes The Nodes in the graph
   * @param arcs A map from each node to all the nodes that depend on it.
   * @note It is assumed that the set of values in nodes and the set of
   *       values in keys and values in arcs must be equal.
   * @note Each node is assumed to have a unique value
   */
  template <typename Value>
  bool topologicalSort(Nodes<Value>& nodes, const Arcs<Value>& arcs) {
    // Use Kahn's Algorithm to do a topological sort
    std::unordered_map<Value, size_t> inDegree;
    std::unordered_map<Value, std::vector<Value>> dependentArcs;

    std::ranges::for_each(nodes, [&](const Value& node) { inDegree.insert({node, 0}); });

    // Calculate in-degrees and dependents
    for (const auto& [source, targets] : arcs) {
      for (const auto& target : targets) {
        ++inDegree[target];
        dependentArcs[source].push_back(target);
      }
    }

    // Find all root nodes
    std::deque<Value> ready;
    for (const auto& node : nodes) {
      if (inDegree[node] == 0) {
        ready.push_back(node);
      }
    }

    // Process nodes in order until we run out of ready nodes
    auto b = nodes.begin();

    while (!ready.empty()) {
      Value current = ready.front();
      ready.pop_front();
      *b++ = current;

      // Reduce in degree of dependent nodes
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

  /** Sorts the directed graph topologically, if possible.
   *
   * @tparam Value The value type of each node
   * @tparam IdFunc A function mapping from Value to an ID type used in arcs
   * @param nodes The nodes of the graph
   * @param arcs A map from each node id to all the node ids that depend on it.
   * @param getId A function to map from Value type to ID type
   * @return true if a topological sort exists, false otherwise
   */
  template <typename Value, typename IdFunc>
  bool topologicalSort(Nodes<Value>& nodes,
                       const Arcs<std::invoke_result_t<IdFunc, const Value&>>& arcs,
                       IdFunc getId) {
    using Id = std::invoke_result_t<IdFunc, const Value&>;

    // Build a mapping of the IDs to the original nodes
    // Create a vector of the ID types, and sort that vector
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

    // Use the sorted ids to reorder the output vector
    // TODO: This could be made slightly more efficient by performing it in-place...
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
