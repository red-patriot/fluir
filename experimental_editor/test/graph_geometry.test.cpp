#include "editor/core/graph_geometry.hpp"

#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/geometry.hpp"

namespace {

  using fluir::FlowGraphLocation;
  using fluir::ID;
  using fluir::editor::atOrigin;
  using fluir::editor::bodyOrigin;
  using fluir::editor::dotRect;
  using fluir::editor::localRect;
  using fluir::editor::Rect;
  using fluir::editor::sortedFunctions;
  using fluir::editor::sortedNodes;
  using fluir::editor::Vec2;

  // Builds a bare-bones FunctionDecl at the given (id, z), for ordering tests
  // only — geometry fields are irrelevant here.
  fluir::pt::FunctionDecl makeFunction(ID id, int z) {
    fluir::pt::FunctionDecl fn;
    fn.id = id;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = z, .width = 0, .height = 0};
    fn.name = "f";
    return fn;
  }

  // Builds a bare-bones Constant node at the given (id, z), for ordering tests
  // only — geometry fields are irrelevant here.
  fluir::pt::Node makeNode(ID id, int z) {
    fluir::pt::Constant constant;
    constant.id = id;
    constant.location = FlowGraphLocation{.x = 0, .y = 0, .z = z, .width = 0, .height = 0};
    constant.value = fluir::literals_types::I32{0};
    return constant;
  }

}  // namespace

TEST(GraphGeometry, LocalRectScalesLocationByUnitPx) {
  // int_constants.fl constant id=1: x=2 y=20 w=5 h=5, unitPx=5.
  const FlowGraphLocation loc{.x = 2, .y = 20, .z = 1, .width = 5, .height = 5};
  EXPECT_EQ(localRect(loc, 5.0), (Rect{10, 100, 25, 25}));
}

TEST(GraphGeometry, BodyOriginOffsetsByHeaderHeight) {
  const Vec2 origin{50, 50};
  EXPECT_EQ(bodyOrigin(origin, 25.0), (Vec2{50, 75}));
}

TEST(GraphGeometry, AtOriginShiftsLocalRectKeepingSize) {
  // Matches graph_draw.test.cpp's ConstantNode assertion for constant id=1.
  const Vec2 body{50, 75};
  const Rect local{10, 100, 25, 25};
  EXPECT_EQ(atOrigin(body, local), (Rect{60, 175, 25, 25}));
}

TEST(GraphGeometry, DotRectIsCenteredOnAnchor) { EXPECT_EQ(dotRect(Vec2{122, 84.5}, 6.0), (Rect{119, 81.5, 6, 6})); }

TEST(GraphGeometry, SortedFunctionsOrdersByZThenId) {
  fluir::pt::ParseTree tree;
  tree.declarations.emplace(1, fluir::pt::Declaration{makeFunction(1, 2)});
  tree.declarations.emplace(2, fluir::pt::Declaration{makeFunction(2, 1)});
  tree.declarations.emplace(3, fluir::pt::Declaration{makeFunction(3, 1)});  // ties z=1 with id=2, lower id first

  const std::vector<const fluir::pt::FunctionDecl*> sorted = sortedFunctions(tree);
  ASSERT_EQ(sorted.size(), 3u);
  EXPECT_EQ(sorted[0]->id, 2u);  // z=1, id=2
  EXPECT_EQ(sorted[1]->id, 3u);  // z=1, id=3
  EXPECT_EQ(sorted[2]->id, 1u);  // z=2, id=1
}

TEST(GraphGeometry, SortedNodesOrdersByZThenId) {
  fluir::pt::Block block;
  block.nodes.emplace(1, makeNode(1, 2));
  block.nodes.emplace(2, makeNode(2, 1));
  block.nodes.emplace(3, makeNode(3, 1));  // ties z=1 with id=2, lower id first

  const auto idOf = [](const fluir::pt::Node* n) { return std::visit([](const auto& node) { return node.id; }, *n); };

  const std::vector<const fluir::pt::Node*> sorted = sortedNodes(block);
  ASSERT_EQ(sorted.size(), 3u);
  EXPECT_EQ(idOf(sorted[0]), 2u);  // z=1, id=2
  EXPECT_EQ(idOf(sorted[1]), 3u);  // z=1, id=3
  EXPECT_EQ(idOf(sorted[2]), 1u);  // z=2, id=1
}
