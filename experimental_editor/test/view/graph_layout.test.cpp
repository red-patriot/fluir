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
  using fluir::editor::railAt;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using testutil::expectRectNear;
  using testutil::loadFixture;

  const EditorContext kCtx;

  fluir::pt::Constant makeConstant(ID id,
                                   FlowGraphLocation loc,
                                   fluir::literals_types::Literal value = fluir::literals_types::I32{0}) {
    return fluir::pt::Constant{.id = id, .location = loc, .value = value};
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

// A node dragged past the frame's bottom edge is clipped there, not a header-height lower.
TEST(GraphLayout, BodyClipEndsAtTheFrameBottom) {
  fluir::pt::FunctionDecl fn = makeFunction(1);
  fn.body.nodes.emplace(10, makeConstant(10, {.x = 1, .y = 94, .z = 1, .width = 10, .height = 10}));  // world y 495

  const std::vector<Box> boxes = layoutGraph(treeOf({fn}), kCtx.layout);

  ASSERT_NE(pathAt(boxes, Vec2{12, 497}), nullptr);
  EXPECT_EQ(*pathAt(boxes, Vec2{12, 497}), (FullID{1, 10}));
  EXPECT_EQ(pathAt(boxes, Vec2{12, 510}), nullptr);
}

TEST(GraphLayout, NodeGripsAreHittable) {
  const std::vector<Box> boxes = layoutGraph(overlappingNodes(1, 2), kCtx.layout);

  // Node 11 {10,35,50,50}: move grip {40,40,15,15}; resize bar {55,35,5,25}.
  const Box* move = hitAt(boxes, Vec2{45, 45});
  const Box* resize = hitAt(boxes, Vec2{57, 40});

  ASSERT_NE(move, nullptr);
  ASSERT_NE(resize, nullptr);
  EXPECT_EQ(move->part, Part::MoveGrip);
  EXPECT_EQ(move->path, (FullID{1, 11}));
  EXPECT_EQ(resize->part, Part::ResizeX);
  EXPECT_EQ(resize->path, (FullID{1, 11}));
}

TEST(GraphLayout, BoolConstantHasNoResizeGrip) {
  fluir::pt::FunctionDecl fn = makeFunction(1);
  fn.body.nodes.emplace(
    10, makeConstant(10, {.x = 2, .y = 2, .z = 1, .width = 10, .height = 10}, fluir::literals_types::BOOL{true}));

  const std::vector<Box> boxes = layoutGraph(treeOf({fn}), kCtx.layout);

  // Node {10,35,50,50}: the right-edge bar {55,35,5,50} is body, the move grip {40,40,15,15} still grips.
  const Box* edge = hitAt(boxes, Vec2{57, 70});
  const Box* move = hitAt(boxes, Vec2{45, 45});

  ASSERT_NE(edge, nullptr);
  ASSERT_NE(move, nullptr);
  EXPECT_EQ(edge->part, Part::Body);
  EXPECT_EQ(edge->path, (FullID{1, 10}));
  EXPECT_EQ(move->part, Part::MoveGrip);
  EXPECT_EQ(move->path, (FullID{1, 10}));
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

TEST(GraphLayout, RailAtFindsAFunctionsRailUnderAPoint) {
  const testutil::Loaded rails = loadFixture("read/function_with_input_only.fl");
  ASSERT_TRUE(rails.result.tree.has_value());
  const std::vector<Box> boxes = layoutGraph(*rails.result.tree, kCtx.layout);

  const Box* onRail = railAt(boxes, FullID{1}, Vec2{60, 90});  // param a rail {50,75,75,25}

  ASSERT_NE(onRail, nullptr);
  EXPECT_EQ(onRail->path, (FullID{1, 2}));
  EXPECT_EQ(railAt(boxes, FullID{1}, Vec2{300, 300}), nullptr);
  EXPECT_EQ(railAt(boxes, FullID{2}, Vec2{60, 90}), nullptr);
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
  // Move grip {155,55,15,15}; resize corner {160,160,15,15}.
  const Box* move = hitAt(boxes, Vec2{160, 60});
  const Box* resize = hitAt(boxes, Vec2{167, 167});
  const Box* aboveCorner = hitAt(boxes, Vec2{172, 100});

  ASSERT_NE(move, nullptr);
  ASSERT_NE(resize, nullptr);
  ASSERT_NE(aboveCorner, nullptr);
  EXPECT_EQ(move->part, Part::MoveGrip);
  EXPECT_EQ(move->path, (FullID{1}));
  EXPECT_EQ(resize->part, Part::ResizeXY);
  EXPECT_EQ(resize->path, (FullID{1}));
  EXPECT_EQ(aboveCorner->part, Part::Body);
}

TEST(GraphLayout, InBodyCommentHasACornerResizeGrip) {
  const testutil::Loaded l = loadFixture("read/in_body_comment_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);
  // Comment {25,50,125,125}; resize corner {135,160,15,15}.
  const Box* resize = hitAt(boxes, Vec2{142, 167});

  ASSERT_NE(resize, nullptr);
  EXPECT_EQ(resize->part, Part::ResizeXY);
  EXPECT_EQ(resize->path, (FullID{1, 2}));
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

// simple_binary_expr.fl: binary 1 {125,85,25,25} has inputs (125,85), (125,110) and output (150,97.5);
// constant 2 {60,85,25,25} has output (85,97.5). Port hit squares are 5 world px.
TEST(PortAt, FindsNodeInputsAndOutputs) {
  const testutil::Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());
  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);

  const auto in0 = fluir::editor::portAt(*l.result.tree, boxes, Vec2{126, 86}, kCtx.layout);
  const auto in1 = fluir::editor::portAt(*l.result.tree, boxes, Vec2{124, 111}, kCtx.layout);
  const auto out = fluir::editor::portAt(*l.result.tree, boxes, Vec2{151, 98}, kCtx.layout);
  const auto constant = fluir::editor::portAt(*l.result.tree, boxes, Vec2{84, 97}, kCtx.layout);

  ASSERT_TRUE(in0 && in1 && out && constant);
  EXPECT_EQ(in0->path, (FullID{1, 1}));
  EXPECT_FALSE(in0->output);
  EXPECT_EQ(in0->index, 0);
  testutil::expectVecNear(in0->anchor, Vec2{125, 85});
  EXPECT_FALSE(in1->output);
  EXPECT_EQ(in1->index, 1);
  EXPECT_EQ(out->path, (FullID{1, 1}));
  EXPECT_TRUE(out->output);
  EXPECT_EQ(out->index, 0);
  EXPECT_EQ(constant->path, (FullID{1, 2}));
  EXPECT_TRUE(constant->output);
  testutil::expectVecNear(constant->anchor, Vec2{85, 97.5});
}

// function_with_input_only.fl: param a (id 2) rail {50,75,75,25}, port (125,87.5).
// function_with_output_only.fl: return (id 4) rail {525,75,25,25}, port (525,87.5).
TEST(PortAt, FindsFunctionRailPorts) {
  const testutil::Loaded in = loadFixture("read/function_with_input_only.fl");
  const testutil::Loaded out = loadFixture("read/function_with_output_only.fl");
  ASSERT_TRUE(in.result.tree.has_value());
  ASSERT_TRUE(out.result.tree.has_value());

  const auto param =
    fluir::editor::portAt(*in.result.tree, layoutGraph(*in.result.tree, kCtx.layout), Vec2{124, 88}, kCtx.layout);
  const auto ret =
    fluir::editor::portAt(*out.result.tree, layoutGraph(*out.result.tree, kCtx.layout), Vec2{526, 87}, kCtx.layout);

  ASSERT_TRUE(param && ret);
  EXPECT_EQ(param->path, (FullID{1, 2}));
  EXPECT_TRUE(param->output);
  EXPECT_EQ(param->index, 0);
  testutil::expectVecNear(param->anchor, Vec2{125, 87.5});
  EXPECT_EQ(ret->path, (FullID{1, 4}));
  EXPECT_FALSE(ret->output);
  EXPECT_EQ(ret->index, 0);
  testutil::expectVecNear(ret->anchor, Vec2{525, 87.5});
}

TEST(PortAt, MissesAwayFromAnchors) {
  const testutil::Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());
  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);

  EXPECT_FALSE(fluir::editor::portAt(*l.result.tree, boxes, Vec2{105, 97.5}, kCtx.layout));
  EXPECT_FALSE(fluir::editor::portAt(*l.result.tree, boxes, Vec2{137, 97}, kCtx.layout)) << "binary grip";
  EXPECT_FALSE(fluir::editor::portAt(*l.result.tree, boxes, Vec2{-500, -500}, kCtx.layout));
}

// A constant at body units (1,-6) is {5,-5,50,50}: its output (55,20) sits above the body clip at y 25.
TEST(PortAt, HonoursTheBodyClip) {
  fluir::pt::FunctionDecl hidden = makeFunction(1);
  hidden.body.nodes.emplace(10, makeConstant(10, {.x = 1, .y = -6, .z = 1, .width = 10, .height = 10}));
  fluir::pt::FunctionDecl shown = makeFunction(1);
  shown.body.nodes.emplace(10, makeConstant(10, {.x = 1, .y = -3, .z = 1, .width = 10, .height = 10}));
  const fluir::pt::ParseTree hiddenTree = treeOf({hidden});
  const fluir::pt::ParseTree shownTree = treeOf({shown});

  EXPECT_FALSE(fluir::editor::portAt(hiddenTree, layoutGraph(hiddenTree, kCtx.layout), Vec2{55, 20}, kCtx.layout));
  EXPECT_TRUE(fluir::editor::portAt(shownTree, layoutGraph(shownTree, kCtx.layout), Vec2{55, 35}, kCtx.layout));
}

TEST(PortAt, APortDoesNotChangeWhatHitAtFinds) {
  const testutil::Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());
  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);

  ASSERT_TRUE(fluir::editor::portAt(*l.result.tree, boxes, Vec2{126, 86}, kCtx.layout));
  const Box* hit = hitAt(boxes, Vec2{126, 86});
  ASSERT_NE(hit, nullptr);
  EXPECT_EQ(hit->part, Part::Body);
  EXPECT_EQ(hit->path, (FullID{1, 1}));
}

namespace {

  fluir::pt::Scope makeScope(ID id, int y, int h, int width) {
    return fluir::pt::Scope{.id = id, .location = {.x = 0, .y = y, .z = 0, .width = width, .height = h}, .body = {}};
  }

  // A conditional whose branches are `thenH` and `elseH` units tall. `height` is what the
  // file claims; the branches are what layout must believe.
  fluir::pt::Conditional makeConditional(ID id, FlowGraphLocation loc, fluir::pt::Scope then, fluir::pt::Scope else_) {
    return fluir::pt::Conditional{.id = id,
                                  .location = loc,
                                  .condition = {},
                                  .inputs = {},
                                  .outputs = {},
                                  .thenScope = xyz::indirect{std::move(then)},
                                  .elseScope = xyz::indirect{std::move(else_)}};
  }

  // Function 1 (frame {0,0,500,500}, body from y 25) holds conditional 20 at units (2,2) 20 wide,
  // with a 12-unit then branch over a 12-unit else branch. Both branches hold a node with id 1.
  // Each branch gives up its own top 25 to its header.
  //   frame  {10,35,100,120}   then {10,35,100,60} (content from y 60)   else {10,95,100,60} (from y 120)
  //   then 1 {15,65,50,50}     else 1 {15,125,20,20}
  fluir::pt::ParseTree nestedTree(int claimedHeight = 24) {
    fluir::pt::Scope then = makeScope(0, 0, 12, 20);
    then.body.nodes.emplace(1, makeConstant(1, {.x = 1, .y = 1, .z = 0, .width = 10, .height = 10}));
    fluir::pt::Scope else_ = makeScope(1, 12, 12, 20);
    else_.body.nodes.emplace(1, makeConstant(1, {.x = 1, .y = 1, .z = 0, .width = 4, .height = 4}));

    fluir::pt::FunctionDecl fn = makeFunction(1);
    fn.body.nodes.emplace(
      20,
      makeConditional(
        20, {.x = 2, .y = 2, .z = 0, .width = 20, .height = claimedHeight}, std::move(then), std::move(else_)));
    return treeOf({fn});
  }

}  // namespace

TEST(GraphLayout, NestedNodesHitAtTheirDepthFourPaths) {
  const std::vector<Box> boxes = layoutGraph(nestedTree(), kCtx.layout);

  const Box* inThen = hitAt(boxes, Vec2{20, 90});
  const Box* inElse = hitAt(boxes, Vec2{17, 127});

  ASSERT_NE(inThen, nullptr);
  ASSERT_NE(inElse, nullptr);
  EXPECT_EQ(inThen->part, Part::Body);
  EXPECT_EQ(inThen->path, (FullID{1, 20, 0, 1}));
  EXPECT_EQ(inElse->part, Part::Body);
  EXPECT_EQ(inElse->path, (FullID{1, 20, 1, 1}));
}

TEST(GraphLayout, TheTwoBranchesIdOneNodesHitAsDifferentPaths) {
  const std::vector<Box> boxes = layoutGraph(nestedTree(), kCtx.layout);

  const Box* inThen = hitAt(boxes, Vec2{20, 90});
  const Box* inElse = hitAt(boxes, Vec2{17, 127});

  ASSERT_NE(inThen, nullptr);
  ASSERT_NE(inElse, nullptr);
  EXPECT_NE(inThen->path, inElse->path);
  expectRectNear(inThen->world, Rect{15, 65, 50, 50});
  expectRectNear(inElse->world, Rect{15, 125, 20, 20});
}

// A node dragged above its branch is clipped by the branch, not by the function body.
TEST(GraphLayout, NestedNodeIsClippedByItsBranch) {
  fluir::pt::Scope then = makeScope(0, 0, 12, 20);
  then.body.nodes.emplace(1, makeConstant(1, {.x = 1, .y = -3, .z = 0, .width = 10, .height = 10}));  // world y 45
  fluir::pt::Scope else_ = makeScope(1, 12, 6, 20);
  fluir::pt::FunctionDecl fn = makeFunction(1);
  fn.body.nodes.emplace(
    20, makeConditional(20, {.x = 2, .y = 2, .z = 0, .width = 20, .height = 18}, std::move(then), std::move(else_)));

  const std::vector<Box> boxes = layoutGraph(treeOf({fn}), kCtx.layout);

  const Box* aboveBranch = hitAt(boxes, Vec2{20, 50});  // over the then branch's own header
  const Box* insideBranch = hitAt(boxes, Vec2{20, 90});

  ASSERT_NE(aboveBranch, nullptr);
  ASSERT_NE(insideBranch, nullptr);
  EXPECT_EQ(aboveBranch->path, (FullID{1, 20, 0}));  // the header is the branch's, not a node's
  EXPECT_EQ(insideBranch->path, (FullID{1, 20, 0, 1}));
}

// The else branch has a header of its own now, so it clips its nodes exactly as the then branch does.
TEST(GraphLayout, ElseBranchNodeIsClippedByItsOwnHeader) {
  fluir::pt::Scope then = makeScope(0, 0, 12, 20);
  fluir::pt::Scope else_ = makeScope(1, 12, 12, 20);
  else_.body.nodes.emplace(1, makeConstant(1, {.x = 1, .y = -3, .z = 0, .width = 10, .height = 10}));  // world y 105
  fluir::pt::FunctionDecl fn = makeFunction(1);
  fn.body.nodes.emplace(
    20, makeConditional(20, {.x = 2, .y = 2, .z = 0, .width = 20, .height = 24}, std::move(then), std::move(else_)));

  const std::vector<Box> boxes = layoutGraph(treeOf({fn}), kCtx.layout);

  const Box* aboveBranch = hitAt(boxes, Vec2{20, 110});  // over the else branch's own header
  const Box* insideBranch = hitAt(boxes, Vec2{20, 130});

  ASSERT_NE(aboveBranch, nullptr);
  ASSERT_NE(insideBranch, nullptr);
  EXPECT_EQ(aboveBranch->path, (FullID{1, 20, 1}));  // just under the divider: the scope, not the node
  EXPECT_EQ(insideBranch->path, (FullID{1, 20, 1, 1}));
}

TEST(GraphLayout, AnEmptyBranchHitCarriesItsScopePath) {
  const std::vector<Box> boxes = layoutGraph(nestedTree(), kCtx.layout);

  const Box* inThen = hitAt(boxes, Vec2{90, 90});   // clear of the then branch's only node
  const Box* inElse = hitAt(boxes, Vec2{90, 130});  // below the else branch's own header

  ASSERT_NE(inThen, nullptr);
  ASSERT_NE(inElse, nullptr);
  EXPECT_EQ(inThen->part, Part::Scope);
  EXPECT_EQ(inThen->path, (FullID{1, 20, 0}));
  EXPECT_EQ(inElse->part, Part::Scope);
  EXPECT_EQ(inElse->path, (FullID{1, 20, 1}));
}

// The branches are authoritative: a conditional spans exactly the two of them, whatever
// its own location claims.
TEST(GraphLayout, ConditionalSpansBothBranches) {
  const std::vector<Box> tooShort = layoutGraph(nestedTree(3), kCtx.layout);
  const std::vector<Box> tooTall = layoutGraph(nestedTree(80), kCtx.layout);

  for (const std::vector<Box>& boxes : {tooShort, tooTall}) {
    const Box* inElse = hitAt(boxes, Vec2{17, 127});
    const Box* branchBottom = hitAt(boxes, Vec2{90, 150});
    const Box* belowIt = hitAt(boxes, Vec2{90, 165});

    ASSERT_NE(inElse, nullptr);
    ASSERT_NE(branchBottom, nullptr);
    ASSERT_NE(belowIt, nullptr);
    EXPECT_EQ(inElse->path, (FullID{1, 20, 1, 1}));
    EXPECT_EQ(branchBottom->path, (FullID{1, 20, 1}));
    EXPECT_EQ(belowIt->path, (FullID{1}));  // past the conditional's derived bottom edge
  }
}

TEST(GraphLayout, ConditionalGripsAreHittableOverItsBranches) {
  const std::vector<Box> boxes = layoutGraph(nestedTree(), kCtx.layout);

  // Frame {10,35,100,120}: then-header move grip {90,40,15,15}; width bar {105,35,5,25}; corner {95,140,15,15}.
  const Box* move = hitAt(boxes, Vec2{95, 45});
  const Box* bar = hitAt(boxes, Vec2{107, 45});
  const Box* corner = hitAt(boxes, Vec2{100, 145});

  ASSERT_NE(move, nullptr);
  ASSERT_NE(bar, nullptr);
  ASSERT_NE(corner, nullptr);
  EXPECT_EQ(move->part, Part::MoveGrip);
  EXPECT_EQ(move->path, (FullID{1, 20}));
  EXPECT_EQ(bar->part, Part::ResizeX);
  EXPECT_EQ(bar->path, (FullID{1, 20}));
  // The corner resizes the bottom branch, which is what makes the conditional taller.
  EXPECT_EQ(corner->part, Part::ResizeXY);
  EXPECT_EQ(corner->path, (FullID{1, 20, 1}));
}

TEST(GraphLayout, NestingDoesNotChangeGraphBounds) {
  const std::vector<Box> plain = layoutGraph(treeOf({makeFunction(1)}), kCtx.layout);
  const std::vector<Box> nested = layoutGraph(nestedTree(), kCtx.layout);

  expectRectNear(graphBounds(nested), graphBounds(plain));
  expectRectNear(graphBounds(nested), Rect{0, 0, 500, 500});
}

// Node ids repeat across branches, so each block resolves its conduits against its own
// nodes: a dangling endpoint in one branch must not latch onto a namesake in another.
TEST(GraphLayout, ConduitEndpointsResolveWithinTheirOwnBlock) {
  fluir::pt::Scope then = makeScope(0, 0, 12, 20);
  then.body.nodes.emplace(1, makeConstant(1, {.x = 1, .y = 1, .z = 0, .width = 4, .height = 4}));
  then.body.nodes.emplace(
    2,
    fluir::pt::Unary{
      .id = 2, .location = {.x = 8, .y = 1, .z = 0, .width = 4, .height = 4}, .lhs = 1, .op = fluir::Operator::BANG});
  then.body.conduits.emplace(50, fluir::pt::Conduit{.id = 50, .input = 1, .children = {{.target = 2, .index = 0}}});

  fluir::pt::Scope else_ = makeScope(1, 12, 6, 20);
  else_.body.nodes.emplace(1, makeConstant(1, {.x = 1, .y = 1, .z = 0, .width = 4, .height = 4}));
  else_.body.conduits.emplace(60, fluir::pt::Conduit{.id = 60, .input = 1, .children = {{.target = 2, .index = 0}}});

  fluir::pt::FunctionDecl fn = makeFunction(1);
  fn.body.nodes.emplace(
    20, makeConditional(20, {.x = 2, .y = 2, .z = 0, .width = 20, .height = 18}, std::move(then), std::move(else_)));

  const std::vector<Box> boxes = layoutGraph(treeOf({fn}), kCtx.layout);

  bool thenWire = false;
  bool elseWire = false;
  for (const Box& box : boxes) {
    if (box.part != Part::Wire) continue;
    thenWire = thenWire || box.path == FullID{1, 20, 0, 50};
    elseWire = elseWire || box.path == FullID{1, 20, 1, 60};
  }
  EXPECT_TRUE(thenWire);
  EXPECT_FALSE(elseWire);  // node 2 lives in the other branch
}

// conditional_with_body.fl: function 1 {0,0,5000,5000}, body from y 25, holds constant 1
// {30,75,40,25} and conditional 2 -> frame {50,40,2500,2500}.
//   then branch {50,40,2500,1250}, content from y 65: constant 2 {75,90,25,25}, binary 1 {50,165,35,35}
//   else branch {50,1290,2500,1250}, content from y 1315: constant 1 {95,1340,60,25}, binary 3 {225,1330,25,25}
TEST(GraphLayout, FixtureConditionalLaysOutBothBranches) {
  const testutil::Loaded l = loadFixture("read/conditional_with_body.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);
  const Box* inThen = hitAt(boxes, Vec2{80, 95});     // constant 2, clear of its grips
  const Box* inElse = hitAt(boxes, Vec2{100, 1345});  // constant 1, clear of its grips
  const Box* emptyElse = hitAt(boxes, Vec2{2000, 2000});

  ASSERT_NE(inThen, nullptr);
  ASSERT_NE(inElse, nullptr);
  ASSERT_NE(emptyElse, nullptr);
  EXPECT_EQ(inThen->path, (FullID{1, 2, 0, 2}));
  EXPECT_EQ(inElse->path, (FullID{1, 2, 1, 1}));
  EXPECT_EQ(emptyElse->path, (FullID{1, 2, 1}));
}

// The fixture reuses id 1 for a node in the function body, one in the else branch, and the
// else scope itself: only the path tells them apart.
TEST(GraphLayout, FixtureIdOneResolvesPerBlock) {
  const testutil::Loaded l = loadFixture("read/conditional_with_body.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);
  const Box* inFunction = hitAt(boxes, Vec2{35, 80});
  const Box* inElse = hitAt(boxes, Vec2{100, 1345});

  ASSERT_NE(inFunction, nullptr);
  ASSERT_NE(inElse, nullptr);
  EXPECT_EQ(inFunction->path, (FullID{1, 1}));
  EXPECT_EQ(inElse->path, (FullID{1, 2, 1, 1}));
  expectRectNear(inFunction->world, Rect{30, 75, 40, 25});
  expectRectNear(inElse->world, Rect{95, 1340, 60, 25});
}

// conditional_empty_scopes.fl: conditional 2 at units (10,3) 100 wide, then 60 over else 40.
//   frame {50,40,500,500}   then {50,40,500,300}   else {50,340,500,200} (content from y 365)
TEST(GraphLayout, EmptyBranchesStillLayOutAndSpanTheirConditional) {
  const testutil::Loaded l = loadFixture("read/conditional_empty_scopes.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);
  const Box* inThen = hitAt(boxes, Vec2{300, 200});
  const Box* inElse = hitAt(boxes, Vec2{300, 400});
  const Box* belowIt = hitAt(boxes, Vec2{300, 560});

  ASSERT_NE(inThen, nullptr);
  ASSERT_NE(inElse, nullptr);
  ASSERT_NE(belowIt, nullptr);
  EXPECT_EQ(inThen->path, (FullID{1, 2, 0}));
  EXPECT_EQ(inElse->path, (FullID{1, 2, 1}));
  EXPECT_EQ(belowIt->path, (FullID{1}));  // the conditional ends where its branches do
}
