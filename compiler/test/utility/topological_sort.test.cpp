#include "compiler/utility/topological_sort.hpp"

#include <string>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

TEST(TestTopologicalSort, SortsGraphWithNoCycles) {
  std::vector<int> expected{3, 2, 1};

  std::vector<int> nodes{1, 2, 3};
  std::unordered_map<int, std::vector<int>> edges{{3, {2}}, {2, {1}}, {1, {}}};

  EXPECT_TRUE(fluir::topologicalSort(nodes, edges));

  EXPECT_EQ(expected, nodes);
}

TEST(TestTopologicalSort, SortsGraphWithMultipleRoots) {
  std::vector<int> expected{1, 5, 2, 3, 4};

  std::vector<int> nodes{1, 2, 3, 4, 5};
  std::unordered_map<int, std::vector<int>> edges{{1, {2, 3}}, {2, {3}}, {3, {4}}, {4, {}}, {5, {3, 4}}};

  EXPECT_TRUE(fluir::topologicalSort(nodes, edges));

  EXPECT_EQ(expected, nodes);
}

TEST(TestTopologicalSort, FailsIfThereIsACycle) {
  std::vector<int> nodes{1, 2, 3, 4, 5};
  std::unordered_map<int, std::vector<int>> edges{{1, {2}}, {2, {3}}, {3, {4}}, {4, {2}}, {5, {4}}};

  EXPECT_FALSE(fluir::topologicalSort(nodes, edges));
}

TEST(TestTopologicalSort, FailsIfWholeGraphIsACycle) {
  std::vector<int> nodes{1, 2, 3, 4, 5};
  std::unordered_map<int, std::vector<int>> edges{{1, {2}}, {2, {3}}, {3, {4}}, {4, {5}}, {5, {1}}};

  EXPECT_FALSE(fluir::topologicalSort(nodes, edges));
}

TEST(TestTopologicalSort, StressTest) {
  constexpr size_t SIZE = 10000;
  std::vector<int> expected(SIZE);
  std::ranges::iota(expected, 0);
  std::vector<int> nodes = expected;
  std::ranges::reverse(nodes);

  std::unordered_map<int, std::vector<int>> edges{};
  for (const auto& node : nodes) {
    std::vector<int> children;
    for (int i = node + 1; i < SIZE && i < node + 6; ++i) {
      children.push_back(i);
    }
    edges.emplace(node, std::move(children));
  }

  EXPECT_TRUE(fluir::topologicalSort(nodes, edges));

  EXPECT_EQ(expected, nodes);
}
