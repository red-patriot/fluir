#include "compiler/utility/topological_sort.hpp"

#include <memory>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

TEST(TestTopologicalSort, SortsGraphWithNoCycles) {
  std::vector<int> expected{3, 2, 1};

  fluir::dag::Nodes<int> nodes{1, 2, 3};
  fluir::dag::Arcs<int> arcs{{1, {2}}, {2, {3}}, {3, {}}};

  EXPECT_TRUE(fluir::dag::topologicalSort(nodes, arcs));

  EXPECT_EQ(expected, nodes);
}

TEST(TestTopologicalSort, SortsGraphWithMultipleRoots) {
  fluir::dag::Nodes<int> expected{1, 5, 2, 3, 4};

  fluir::dag::Nodes<int> nodes{1, 2, 3, 4, 5};
  fluir::dag::Arcs<int> arcs{{1, {}}, {2, {1}}, {3, {1, 2, 5}}, {4, {3, 5}}, {5, {}}};

  EXPECT_TRUE(fluir::dag::topologicalSort(nodes, arcs));

  EXPECT_EQ(expected, nodes);
}

TEST(TestTopologicalSort, FailsIfThereIsACycle) {
  fluir::dag::Nodes<int> nodes{1, 2, 3, 4, 5};
  fluir::dag::Arcs<int> arcs{{1, {}}, {2, {4}}, {3, {2}}, {4, {3, 5}}, {5, {}}};

  EXPECT_FALSE(fluir::dag::topologicalSort(nodes, arcs));
}

TEST(TestTopologicalSort, FailsIfWholeGraphIsACycle) {
  fluir::dag::Nodes<int> nodes{1, 2, 3, 4, 5};
  fluir::dag::Arcs<int> arcs{{1, {2}}, {2, {3}}, {3, {4}}, {4, {5}}, {5, {1}}};

  EXPECT_FALSE(fluir::dag::topologicalSort(nodes, arcs));
}

TEST(TestTopologicalSort, StressTest) {
  constexpr size_t SIZE = 10000;
  fluir::dag::Nodes<int> expected(SIZE);
  std::ranges::iota(expected, 0);
  fluir::dag::Nodes<int> nodes = expected;
  std::ranges::reverse(nodes);

  fluir::dag::Arcs<int> arcs{};
  for (const auto& node : nodes) {
    fluir::dag::NodeSet<int> children;
    for (int i = std::max(0, node - 5); i < node; ++i) {
      children.insert(i);
    }
    arcs.emplace(node, std::move(children));
  }

  EXPECT_TRUE(fluir::dag::topologicalSort(nodes, arcs));

  EXPECT_EQ(expected, nodes);
}

TEST(TestTopologicalSort, HandlesIdMappingCorrectly) {
  using namespace std::string_literals;
  fluir::dag::Nodes<int> expected{2, 3, 1};

  fluir::dag::Nodes<int> nodes{1, 2, 3};
  fluir::dag::Arcs<std::string> arcs{{"1"s, {"3"s}}, {"2"s, {}}, {"3"s, {"2"s}}};

  EXPECT_TRUE(fluir::dag::topologicalSort(nodes, arcs, [](int node) -> std::string { return std::to_string(node); }));

  EXPECT_EQ(expected, nodes);
}

TEST(TestTopologicalSort, MappedVersionFailsIfThereIsACycle) {
  fluir::dag::Nodes<int> nodes{1, 2, 3, 4, 5};
  fluir::dag::Arcs<std::string> arcs{{"1", {"2"}}, {"2", {"3"}}, {"3", {"4"}}, {"4", {"2"}}, {"5", {"4"}}};

  EXPECT_FALSE(fluir::dag::topologicalSort(nodes, arcs, [](int node) -> std::string { return std::to_string(node); }));
}

TEST(TestTopologicalSort, HandlesMoveOnlyTypes) {
  fluir::dag::Nodes<int> expected{5, 1, 3};

  fluir::dag::Nodes<std::unique_ptr<int>> nodes{};
  nodes.emplace_back(std::make_unique<int>(1));
  nodes.emplace_back(std::make_unique<int>(3));
  nodes.emplace_back(std::make_unique<int>(5));

  fluir::dag::Arcs<int> arcs{{1, {5}}, {3, {1}}, {5, {}}};

  EXPECT_TRUE(fluir::dag::topologicalSort(nodes, arcs, [](const std::unique_ptr<int>& node) -> int { return *node; }));

  EXPECT_TRUE(
    std::ranges::equal(expected, nodes, [](int lhs, const std::unique_ptr<int>& rhs) { return lhs == *rhs; }));
}

TEST(TestTopologicalSort, HandlesEmptyGraph) {
  fluir::dag::Nodes<std::string> nodes{};
  fluir::dag::Arcs<std::string> arcs{};

  EXPECT_TRUE(fluir::dag::topologicalSort(nodes, arcs));
}
