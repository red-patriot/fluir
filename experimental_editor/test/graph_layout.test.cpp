#include "editor/view/graph_layout.hpp"

#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "fixture_loader.hpp"
#include "recording_renderer.hpp"

// Layout contracts, asserted through what a point hits and what the graph spans.
// Body content sits below the header band: body y = frame y + 25 world px.

namespace {

  using fluir::FlowGraphLocation;
  using fluir::FullID;
  using fluir::ID;
  using fluir::editor::Box;
  using fluir::editor::EditorContext;
  using fluir::editor::graphBounds;
  using fluir::editor::hitAt;
  using fluir::editor::layoutGraph;
  using fluir::editor::Part;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using testutil::expectRectNear;
  using testutil::loadFixture;

  const EditorContext kCtx;

  fluir::pt::Constant makeConstant(ID id, FlowGraphLocation loc) {
    return fluir::pt::Constant{.id = id, .location = loc, .value = fluir::literals_types::I32{0}};
  }

  // A 100x100-unit function at the origin: frame {0,0,500,500}, body from y 25.
  fluir::pt::FunctionDecl makeFunction(ID id) {
    fluir::pt::FunctionDecl fn;
    fn.id = id;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    return fn;
  }

  fluir::pt::ParseTree treeOf(std::vector<fluir::pt::FunctionDecl> fns) {
    fluir::pt::ParseTree tree;
    for (auto& fn : fns) {
      tree.declarations.emplace(fn.id, fluir::pt::Declaration{std::move(fn)});
    }
    return tree;
  }

  const FullID* pathAt(const std::vector<Box>& boxes, Vec2 world) {
    const Box* box = hitAt(boxes, world);
    return box == nullptr ? nullptr : &box->path;
  }

  // Node 10 at (1,1) and node 11 at (2,2), both 10x10 units: world {5,30,50,50} and {10,35,50,50}.
  fluir::pt::ParseTree overlappingNodes(int z10, int z11) {
    fluir::pt::FunctionDecl fn = makeFunction(1);
    fn.body.nodes.emplace(10, makeConstant(10, {.x = 1, .y = 1, .z = z10, .width = 10, .height = 10}));
    fn.body.nodes.emplace(11, makeConstant(11, {.x = 2, .y = 2, .z = z11, .width = 10, .height = 10}));
    return treeOf({fn});
  }

}  // namespace

TEST(GraphLayout, FixtureConstantLaysOutAtItsVerifiedWorldRect) {
  const testutil::Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);
  const Box* hit = hitAt(boxes, Vec2{62, 198});  // constant -5, clear of its grips

  ASSERT_NE(hit, nullptr);
  EXPECT_EQ(hit->part, Part::Body);
  EXPECT_EQ(hit->path.size(), 2u);
  expectRectNear(hit->world, Rect{60, 175, 25, 25});
}

TEST(GraphLayout, HitPrefersTheHigherZOnOverlap) {
  const std::vector<Box> below = layoutGraph(overlappingNodes(1, 2), kCtx.layout);
  const std::vector<Box> above = layoutGraph(overlappingNodes(3, 2), kCtx.layout);

  ASSERT_NE(pathAt(below, Vec2{12, 45}), nullptr);
  ASSERT_NE(pathAt(above, Vec2{12, 45}), nullptr);
  EXPECT_EQ(*pathAt(below, Vec2{12, 45}), (FullID{1, 11}));
  EXPECT_EQ(*pathAt(above, Vec2{12, 45}), (FullID{1, 10}));
}

TEST(GraphLayout, HitOutsideEverythingIsNull) {
  const std::vector<Box> boxes = layoutGraph(overlappingNodes(1, 2), kCtx.layout);

  EXPECT_EQ(hitAt(boxes, Vec2{-500, -500}), nullptr);
}

TEST(GraphLayout, HitOnEmptyFrameAreaIsTheFunction) {
  const std::vector<Box> boxes = layoutGraph(overlappingNodes(1, 2), kCtx.layout);

  ASSERT_NE(pathAt(boxes, Vec2{300, 300}), nullptr);
  EXPECT_EQ(*pathAt(boxes, Vec2{300, 300}), (FullID{1}));
}

TEST(GraphLayout, NodeIdsAreScopedToTheirFunction) {
  fluir::pt::FunctionDecl a = makeFunction(1);
  a.body.nodes.emplace(2, makeConstant(2, {.x = 1, .y = 1, .z = 1, .width = 10, .height = 10}));
  fluir::pt::FunctionDecl b = makeFunction(2);
  b.location.x = 200;  // world x 1000
  b.body.nodes.emplace(2, makeConstant(2, {.x = 1, .y = 1, .z = 1, .width = 10, .height = 10}));

  const std::vector<Box> boxes = layoutGraph(treeOf({a, b}), kCtx.layout);

  ASSERT_NE(pathAt(boxes, Vec2{7, 50}), nullptr);
  ASSERT_NE(pathAt(boxes, Vec2{1007, 50}), nullptr);
  EXPECT_EQ(*pathAt(boxes, Vec2{7, 50}), (FullID{1, 2}));
  EXPECT_EQ(*pathAt(boxes, Vec2{1007, 50}), (FullID{2, 2}));
}

// A node dragged above the body is clipped there: the header beneath wins.
TEST(GraphLayout, HitRespectsTheBodyClip) {
  fluir::pt::FunctionDecl fn = makeFunction(1);
  fn.body.nodes.emplace(10, makeConstant(10, {.x = 1, .y = -3, .z = 1, .width = 10, .height = 10}));  // world y 10

  const std::vector<Box> boxes = layoutGraph(treeOf({fn}), kCtx.layout);

  ASSERT_NE(pathAt(boxes, Vec2{12, 15}), nullptr);
  EXPECT_EQ(*pathAt(boxes, Vec2{12, 15}), (FullID{1}));
  ASSERT_NE(pathAt(boxes, Vec2{12, 40}), nullptr);
  EXPECT_EQ(*pathAt(boxes, Vec2{12, 40}), (FullID{1, 10}));
}

TEST(GraphLayout, NodeGripsAreHittable) {
  const std::vector<Box> boxes = layoutGraph(overlappingNodes(1, 2), kCtx.layout);

  // Node 11 {10,35,50,50}: move grip {40,40,15,15}; resize bar {55,35,5,50}.
  const Box* move = hitAt(boxes, Vec2{45, 45});
  const Box* resize = hitAt(boxes, Vec2{57, 70});

  ASSERT_NE(move, nullptr);
  ASSERT_NE(resize, nullptr);
  EXPECT_EQ(move->part, Part::MoveGrip);
  EXPECT_EQ(move->path, (FullID{1, 11}));
  EXPECT_EQ(resize->part, Part::ResizeX);
  EXPECT_EQ(resize->path, (FullID{1, 11}));
}

TEST(GraphLayout, FunctionGripsAreHittableAndDrawnOverTheBody) {
  fluir::pt::FunctionDecl fn = makeFunction(1);
  // A node under the corner grip: the overlay grip still wins.
  fn.body.nodes.emplace(10, makeConstant(10, {.x = 90, .y = 90, .z = 1, .width = 10, .height = 10}));

  const std::vector<Box> boxes = layoutGraph(treeOf({fn}), kCtx.layout);

  // Frame {0,0,500,500}: header move grip {480,5,15,15}; corner {485,485,15,15}.
  const Box* move = hitAt(boxes, Vec2{485, 10});
  const Box* corner = hitAt(boxes, Vec2{490, 490});

  ASSERT_NE(move, nullptr);
  ASSERT_NE(corner, nullptr);
  EXPECT_EQ(move->part, Part::MoveGrip);
  EXPECT_EQ(move->path, (FullID{1}));
  EXPECT_EQ(corner->part, Part::ResizeXY);
  EXPECT_EQ(corner->path, (FullID{1}));
}

TEST(GraphLayout, RailsAndWiresAreNotHittable) {
  const testutil::Loaded rails = loadFixture("read/function_with_input_only.fl");
  const testutil::Loaded wires = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(rails.result.tree.has_value());
  ASSERT_TRUE(wires.result.tree.has_value());

  const std::vector<Box> railBoxes = layoutGraph(*rails.result.tree, kCtx.layout);
  const std::vector<Box> wireBoxes = layoutGraph(*wires.result.tree, kCtx.layout);

  const Box* onRail = hitAt(railBoxes, Vec2{60, 90});      // param a rail {50,75,75,25}
  const Box* onWire = hitAt(wireBoxes, Vec2{105, 91.25});  // midpoint of (85,97.5)->(125,85)

  ASSERT_NE(onRail, nullptr);
  ASSERT_NE(onWire, nullptr);
  EXPECT_EQ(onRail->path.size(), 1u);
  EXPECT_EQ(onWire->path.size(), 1u);
}

// top_level_comment_only.fl: comment 1 at units (10,10) 25x25, no header offset.
TEST(GraphLayout, TopLevelCommentLaysOutAtItsWorldRect) {
  const testutil::Loaded l = loadFixture("read/top_level_comment_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);
  const Box* hit = hitAt(boxes, Vec2{60, 150});

  ASSERT_NE(hit, nullptr);
  EXPECT_EQ(hit->part, Part::Body);
  EXPECT_EQ(hit->path, (FullID{1}));
  EXPECT_FALSE(hit->clip.has_value());
  expectRectNear(hit->world, Rect{50, 50, 125, 125});
}

TEST(GraphLayout, TopLevelCommentGripsAreHittable) {
  const testutil::Loaded l = loadFixture("read/top_level_comment_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);
  // Move grip {155,55,15,15}; resize bar {170,50,5,125}.
  const Box* move = hitAt(boxes, Vec2{160, 60});
  const Box* resize = hitAt(boxes, Vec2{172, 150});

  ASSERT_NE(move, nullptr);
  ASSERT_NE(resize, nullptr);
  EXPECT_EQ(move->part, Part::MoveGrip);
  EXPECT_EQ(move->path, (FullID{1}));
  EXPECT_EQ(resize->part, Part::ResizeX);
  EXPECT_EQ(resize->path, (FullID{1}));
}

TEST(GraphLayout, TopLevelDeclarationsInterleaveByZ) {
  const auto treeWith = [](int fnZ) {
    fluir::pt::FunctionDecl fn = makeFunction(1);
    fn.location.z = fnZ;
    fluir::pt::ParseTree tree = treeOf({fn});
    tree.declarations.emplace(2,
                              fluir::pt::Declaration{fluir::pt::Comment{
                                .id = 2, .location = {.x = 10, .y = 10, .z = 1, .width = 25, .height = 25}}});
    return tree;
  };
  const std::vector<Box> commentAbove = layoutGraph(treeWith(0), kCtx.layout);
  const std::vector<Box> functionAbove = layoutGraph(treeWith(5), kCtx.layout);

  ASSERT_NE(pathAt(commentAbove, Vec2{60, 100}), nullptr);
  ASSERT_NE(pathAt(functionAbove, Vec2{60, 100}), nullptr);
  EXPECT_EQ(*pathAt(commentAbove, Vec2{60, 100}), (FullID{2}));
  EXPECT_EQ(*pathAt(functionAbove, Vec2{60, 100}), (FullID{1}));
}

// mixed_comments.fl: body comment 1 at {100,125,125,125}; top-level comment 2 at {5250,50,125,125}.
TEST(GraphLayout, BodyAndTopLevelCommentsAreBothHittable) {
  const testutil::Loaded l = loadFixture("read/mixed_comments.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);

  ASSERT_NE(pathAt(boxes, Vec2{150, 200}), nullptr);
  ASSERT_NE(pathAt(boxes, Vec2{5300, 150}), nullptr);
  EXPECT_EQ(*pathAt(boxes, Vec2{150, 200}), (FullID{1, 1}));
  EXPECT_EQ(*pathAt(boxes, Vec2{5300, 150}), (FullID{2}));
}

TEST(GraphBounds, EmptyTreeIsZero) {
  expectRectNear(graphBounds(layoutGraph(fluir::pt::ParseTree{}, kCtx.layout)), Rect{0, 0, 0, 0});
}

TEST(GraphBounds, SingleFunctionIsItsFrame) {
  const testutil::Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  expectRectNear(graphBounds(layoutGraph(*l.result.tree, kCtx.layout)), Rect{50, 50, 500, 500});
}

TEST(GraphBounds, MultipleFunctionsUnion) {
  const testutil::Loaded l = loadFixture("read/multiple_empty_functions.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  expectRectNear(graphBounds(layoutGraph(*l.result.tree, kCtx.layout)), Rect{50, 50, 2100, 500});
}

TEST(GraphBounds, CoversTopLevelComments) {
  const testutil::Loaded only = loadFixture("read/top_level_comment_only.fl");
  const testutil::Loaded mixed = loadFixture("read/mixed_comments.fl");
  ASSERT_TRUE(only.result.tree.has_value());
  ASSERT_TRUE(mixed.result.tree.has_value());

  expectRectNear(graphBounds(layoutGraph(*only.result.tree, kCtx.layout)), Rect{50, 50, 125, 125});
  expectRectNear(graphBounds(layoutGraph(*mixed.result.tree, kCtx.layout)), Rect{50, 50, 5325, 500});
}

TEST(GraphBounds, ScalesWithUnitPx) {
  const testutil::Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());
  EditorContext ctx;
  ctx.layout.unitPx = 10.0;

  expectRectNear(graphBounds(layoutGraph(*l.result.tree, ctx.layout)), Rect{100, 100, 1000, 1000});
}
