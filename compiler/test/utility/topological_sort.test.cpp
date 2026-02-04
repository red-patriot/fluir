#include "compiler/utility/topological_sort.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

TEST(TestTopologicalSort, SortsGraphWithNoCycles) {
  std::vector<int> expected{3, 2, 1};

  std::vector<int> nodes{1, 2, 3};
  std::unordered_map<int, std::vector<int>> edges{{3, {2}}, {2, {1}}, {1, {}}};

  EXPECT_TRUE(fluir::dag::topologicalSort(nodes, edges));

  EXPECT_EQ(expected, nodes);
}

TEST(TestTopologicalSort, SortsGraphWithMultipleRoots) {
  std::vector<int> expected{1, 5, 2, 3, 4};

  std::vector<int> nodes{1, 2, 3, 4, 5};
  std::unordered_map<int, std::vector<int>> edges{{1, {2, 3}}, {2, {3}}, {3, {4}}, {4, {}}, {5, {3, 4}}};

  EXPECT_TRUE(fluir::dag::topologicalSort(nodes, edges));

  EXPECT_EQ(expected, nodes);
}

TEST(TestTopologicalSort, FailsIfThereIsACycle) {
  std::vector<int> nodes{1, 2, 3, 4, 5};
  std::unordered_map<int, std::vector<int>> edges{{1, {2}}, {2, {3}}, {3, {4}}, {4, {2}}, {5, {4}}};

  EXPECT_FALSE(fluir::dag::topologicalSort(nodes, edges));
}

TEST(TestTopologicalSort, FailsIfWholeGraphIsACycle) {
  std::vector<int> nodes{1, 2, 3, 4, 5};
  std::unordered_map<int, std::vector<int>> edges{{1, {2}}, {2, {3}}, {3, {4}}, {4, {5}}, {5, {1}}};

  EXPECT_FALSE(fluir::dag::topologicalSort(nodes, edges));
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

  EXPECT_TRUE(fluir::dag::topologicalSort(nodes, edges));

  EXPECT_EQ(expected, nodes);
}

TEST(TestTopologicalSort, HandlesIdMappingCorrectly) {
  using namespace std::string_literals;
  fluir::dag::Nodes<int> expected{2, 3, 1};

  fluir::dag::Nodes<int> nodes{1, 2, 3};
  fluir::dag::Arcs<std::string> arcs{{"1"s, {}}, {"2"s, {"3"s}}, {"3"s, {"1"s}}};

  EXPECT_TRUE(fluir::dag::topologicalSort(nodes, arcs, [](int node) -> std::string { return std::to_string(node); }));

  EXPECT_EQ(expected, nodes);
}

TEST(TestTopologicalSort, MappedVersionFailsIfThereIsACycle) {
  fluir::dag::Nodes<int> nodes{1, 2, 3, 4, 5};
  fluir::dag::Arcs<std::string> edges{{"1", {"2"}}, {"2", {"3"}}, {"3", {"4"}}, {"4", {"2"}}, {"5", {"4"}}};

  EXPECT_FALSE(fluir::dag::topologicalSort(nodes, edges, [](int node) -> std::string { return std::to_string(node); }));
}

TEST(TestTopologicalSort, HandlesMoveOnlyTypes) {
  fluir::dag::Nodes<int> expected{5, 1, 3};

  fluir::dag::Nodes<std::unique_ptr<int>> nodes{};
  nodes.emplace_back(std::make_unique<int>(1));
  nodes.emplace_back(std::make_unique<int>(3));
  nodes.emplace_back(std::make_unique<int>(5));

  fluir::dag::Arcs<int> arcs{{1, {3}}, {3, {}}, {5, {1}}};

  EXPECT_TRUE(fluir::dag::topologicalSort(nodes, arcs, [](const std::unique_ptr<int>& node) -> int { return *node; }));

  EXPECT_TRUE(
    std::ranges::equal(expected, nodes, [](int lhs, const std::unique_ptr<int>& rhs) { return lhs == *rhs; }));
}
