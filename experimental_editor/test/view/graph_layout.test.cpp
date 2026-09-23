#include "editor/view/graph_layout.hpp"

#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/tree_path.hpp"
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

  // Node 11 {10,35,50,50}: move grip {40,40,15,15}; right edge {55,35,5,50}, full height.
  const Box* move = hitAt(boxes, Vec2{45, 45});
  const Box* top = hitAt(boxes, Vec2{57, 40});
  const Box* bottom = hitAt(boxes, Vec2{57, 80});

  ASSERT_NE(move, nullptr);
  ASSERT_NE(top, nullptr);
  ASSERT_NE(bottom, nullptr);
  EXPECT_EQ(move->part, Part::MoveGrip);
  EXPECT_EQ(move->path, (FullID{1, 11}));
  EXPECT_EQ(top->part, Part::ResizeX);
  EXPECT_EQ(top->path, (FullID{1, 11}));
  EXPECT_EQ(bottom->part, Part::ResizeX);
  EXPECT_EQ(bottom->path, (FullID{1, 11}));
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

// A comment sizes in both axes, so it keeps the corner handle rather than thick edges.
TEST(GraphLayout, TopLevelCommentHasACornerResizeGrip) {
  const testutil::Loaded l = loadFixture("read/top_level_comment_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);
  // Comment {50,50,125,125}: move grip {155,55,15,15}; resize corner {160,160,15,15}.
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
  EXPECT_EQ(aboveCorner->part, Part::Body) << "no thick right edge on a comment";
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
// constant 2 {60,85,25,25} has output (85,97.5). Terminal hit squares are 5 world px.
TEST(TerminalAt, FindsNodeInputsAndOutputs) {
  const testutil::Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());
  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);

  const auto in0 = fluir::editor::terminalAt(*l.result.tree, boxes, Vec2{126, 86}, kCtx.layout);
  const auto in1 = fluir::editor::terminalAt(*l.result.tree, boxes, Vec2{124, 111}, kCtx.layout);
  const auto out = fluir::editor::terminalAt(*l.result.tree, boxes, Vec2{151, 98}, kCtx.layout);
  const auto constant = fluir::editor::terminalAt(*l.result.tree, boxes, Vec2{84, 97}, kCtx.layout);

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

// function_with_input_only.fl: param a (id 2) rail {50,75,75,25}, terminal (125,87.5).
// function_with_output_only.fl: return (id 4) rail {525,75,25,25}, terminal (525,87.5).
TEST(TerminalAt, FindsFunctionRailTerminals) {
  const testutil::Loaded in = loadFixture("read/function_with_input_only.fl");
  const testutil::Loaded out = loadFixture("read/function_with_output_only.fl");
  ASSERT_TRUE(in.result.tree.has_value());
  ASSERT_TRUE(out.result.tree.has_value());

  const auto param =
    fluir::editor::terminalAt(*in.result.tree, layoutGraph(*in.result.tree, kCtx.layout), Vec2{124, 88}, kCtx.layout);
  const auto ret =
    fluir::editor::terminalAt(*out.result.tree, layoutGraph(*out.result.tree, kCtx.layout), Vec2{526, 87}, kCtx.layout);

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

TEST(TerminalAt, MissesAwayFromAnchors) {
  const testutil::Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());
  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);

  EXPECT_FALSE(fluir::editor::terminalAt(*l.result.tree, boxes, Vec2{105, 97.5}, kCtx.layout));
  EXPECT_FALSE(fluir::editor::terminalAt(*l.result.tree, boxes, Vec2{137, 97}, kCtx.layout)) << "binary grip";
  EXPECT_FALSE(fluir::editor::terminalAt(*l.result.tree, boxes, Vec2{-500, -500}, kCtx.layout));
}

// A constant at body units (1,-6) is {5,-5,50,50}: its output (55,20) sits above the body clip at y 25.
TEST(TerminalAt, HonoursTheBodyClip) {
  fluir::pt::FunctionDecl hidden = makeFunction(1);
  hidden.body.nodes.emplace(10, makeConstant(10, {.x = 1, .y = -6, .z = 1, .width = 10, .height = 10}));
  fluir::pt::FunctionDecl shown = makeFunction(1);
  shown.body.nodes.emplace(10, makeConstant(10, {.x = 1, .y = -3, .z = 1, .width = 10, .height = 10}));
  const fluir::pt::ParseTree hiddenTree = treeOf({hidden});
  const fluir::pt::ParseTree shownTree = treeOf({shown});

  EXPECT_FALSE(fluir::editor::terminalAt(hiddenTree, layoutGraph(hiddenTree, kCtx.layout), Vec2{55, 20}, kCtx.layout));
  EXPECT_TRUE(fluir::editor::terminalAt(shownTree, layoutGraph(shownTree, kCtx.layout), Vec2{55, 35}, kCtx.layout));
}

TEST(TerminalAt, ATerminalDoesNotChangeWhatHitAtFinds) {
  const testutil::Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());
  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);

  ASSERT_TRUE(fluir::editor::terminalAt(*l.result.tree, boxes, Vec2{126, 86}, kCtx.layout));
  const Box* hit = hitAt(boxes, Vec2{126, 86});
  ASSERT_NE(hit, nullptr);
  EXPECT_EQ(hit->part, Part::Body);
  EXPECT_EQ(hit->path, (FullID{1, 1}));
}
namespace {

  using fluir::editor::THEN_BRANCH_ID;

  fluir::pt::Conditional makeConditional(ID id, FlowGraphLocation loc, fluir::pt::Block then, fluir::pt::Block else_) {
    return fluir::pt::Conditional{.id = id,
                                  .location = loc,
                                  .condition = {},
                                  .inputs = {},
                                  .outputs = {},
                                  .thenScope = xyz::indirect{std::move(then)},
                                  .elseScope = xyz::indirect{std::move(else_)}};
  }

  // Function 1 (frame {0,0,500,500}, body from y 25) holds conditional 20 at units (2,2) 20 wide
  // and `height` tall. The conditional is one frame with one header band over its then branch:
  //   frame  {10,35,100,120}   then branch {10,60,100,95}   then node 1 {15,65,50,50}
  fluir::pt::ParseTree nestedTree(int height = 24) {
    fluir::pt::Block then;
    then.nodes.emplace(1, makeConstant(1, {.x = 1, .y = 1, .z = 0, .width = 10, .height = 10}));
    fluir::pt::Block else_;
    else_.nodes.emplace(1, makeConstant(1, {.x = 1, .y = 1, .z = 0, .width = 4, .height = 4}));

    fluir::pt::FunctionDecl fn = makeFunction(1);
    fn.body.nodes.emplace(
      20,
      makeConditional(20, {.x = 2, .y = 2, .z = 0, .width = 20, .height = height}, std::move(then), std::move(else_)));
    return treeOf({fn});
  }

}  // namespace

TEST(GraphLayout, NestedNodesHitAtTheirDepthFourPaths) {
  const std::vector<Box> boxes = layoutGraph(nestedTree(), kCtx.layout);

  const Box* inThen = hitAt(boxes, Vec2{20, 85});

  ASSERT_NE(inThen, nullptr);
  EXPECT_EQ(inThen->part, Part::Body);
  EXPECT_EQ(inThen->path, (FullID{1, 20, THEN_BRANCH_ID, 1}));
  expectRectNear(inThen->world, Rect{15, 65, 50, 50});
}

// A node dragged above the content area is clipped by it, so the header stays the conditional's.
TEST(GraphLayout, NestedNodeIsClippedByTheHeaderBand) {
  fluir::pt::Block then;
  then.nodes.emplace(1, makeConstant(1, {.x = 1, .y = -3, .z = 0, .width = 10, .height = 10}));  // world y 45
  fluir::pt::FunctionDecl fn = makeFunction(1);
  fn.body.nodes.emplace(20,
                        makeConditional(20, {.x = 2, .y = 2, .z = 0, .width = 20, .height = 24}, std::move(then), {}));

  const std::vector<Box> boxes = layoutGraph(treeOf({fn}), kCtx.layout);

  const Box* onHeader = hitAt(boxes, Vec2{20, 50});
  const Box* insideBranch = hitAt(boxes, Vec2{20, 85});

  ASSERT_NE(onHeader, nullptr);
  ASSERT_NE(insideBranch, nullptr);
  EXPECT_EQ(onHeader->path, (FullID{1, 20}));  // the header is the conditional's, not a node's
  EXPECT_EQ(insideBranch->path, (FullID{1, 20, THEN_BRANCH_ID, 1}));
}

TEST(GraphLayout, AHitInTheBodyCarriesTheThenBranchPath) {
  const std::vector<Box> boxes = layoutGraph(nestedTree(), kCtx.layout);

  const Box* inThen = hitAt(boxes, Vec2{90, 85});  // clear of the branch's only node

  ASSERT_NE(inThen, nullptr);
  EXPECT_EQ(inThen->part, Part::Branch);
  EXPECT_EQ(inThen->path, (FullID{1, 20, THEN_BRANCH_ID}));
}

// The conditional owns its height now: the frame is exactly its own location rect.
TEST(GraphLayout, ConditionalSpansItsOwnLocation) {
  const std::vector<Box> shortBoxes = layoutGraph(nestedTree(10), kCtx.layout);  // frame {10,35,100,50}
  const std::vector<Box> tallBoxes = layoutGraph(nestedTree(40), kCtx.layout);   // frame {10,35,100,200}

  // x 90 is clear of the branch's only node, which spans x 15..65.
  EXPECT_EQ(*pathAt(shortBoxes, Vec2{90, 80}), (FullID{1, 20, THEN_BRANCH_ID}));
  EXPECT_EQ(*pathAt(shortBoxes, Vec2{90, 95}), (FullID{1}));  // past the conditional's bottom edge
  EXPECT_EQ(*pathAt(tallBoxes, Vec2{90, 200}), (FullID{1, 20, THEN_BRANCH_ID}));
  EXPECT_EQ(*pathAt(tallBoxes, Vec2{90, 245}), (FullID{1}));
}

TEST(GraphLayout, ConditionalGripsAreHittableOverItsBranch) {
  const std::vector<Box> boxes = layoutGraph(nestedTree(), kCtx.layout);

  // Frame {10,35,100,120}: header move grip {90,40,15,15}; corner grip {95,140,15,15}.
  const Box* move = hitAt(boxes, Vec2{95, 45});
  const Box* corner = hitAt(boxes, Vec2{100, 145});

  ASSERT_NE(move, nullptr);
  ASSERT_NE(corner, nullptr);
  EXPECT_EQ(move->part, Part::MoveGrip);
  EXPECT_EQ(move->path, (FullID{1, 20}));
  EXPECT_EQ(corner->part, Part::ResizeXY);
  EXPECT_EQ(corner->path, (FullID{1, 20}));
  // A branch has no grip of its own: the conditional resizes as one rect.
  for (const Box& box : boxes) {
    EXPECT_NE(box.part, Part::ResizeY);
  }
}

TEST(GraphLayout, NestingDoesNotChangeGraphBounds) {
  const std::vector<Box> plain = layoutGraph(treeOf({makeFunction(1)}), kCtx.layout);
  const std::vector<Box> nested = layoutGraph(nestedTree(), kCtx.layout);

  expectRectNear(graphBounds(nested), graphBounds(plain));
  expectRectNear(graphBounds(nested), Rect{0, 0, 500, 500});
}

// Node ids repeat across blocks, so each block resolves its conduits against its own nodes:
// a dangling endpoint must not latch onto a namesake elsewhere.
TEST(GraphLayout, ConduitEndpointsResolveWithinTheirOwnBlock) {
  fluir::pt::Block then;
  then.nodes.emplace(1, makeConstant(1, {.x = 1, .y = 1, .z = 0, .width = 4, .height = 4}));
  then.nodes.emplace(
    2,
    fluir::pt::Unary{
      .id = 2, .location = {.x = 8, .y = 1, .z = 0, .width = 4, .height = 4}, .lhs = 1, .op = fluir::Operator::BANG});
  then.conduits.emplace(50, fluir::pt::Conduit{.id = 50, .input = 1, .children = {{.target = 2, .index = 0}}});
  then.conduits.emplace(60, fluir::pt::Conduit{.id = 60, .input = 1, .children = {{.target = 99, .index = 0}}});

  fluir::pt::FunctionDecl fn = makeFunction(1);
  fn.body.nodes.emplace(20,
                        makeConditional(20, {.x = 2, .y = 2, .z = 0, .width = 20, .height = 24}, std::move(then), {}));

  const std::vector<Box> boxes = layoutGraph(treeOf({fn}), kCtx.layout);

  bool wired = false;
  bool dangling = false;
  for (const Box& box : boxes) {
    if (box.part != Part::Wire) continue;
    wired = wired || box.path == FullID{1, 20, THEN_BRANCH_ID, 50};
    dangling = dangling || box.path == FullID{1, 20, THEN_BRANCH_ID, 60};
  }
  EXPECT_TRUE(wired);
  EXPECT_FALSE(dangling);  // node 99 is in no block
}

// conditional_with_body.fl: function 1 {0,0,5000,5000}, body from y 25, holds constant 1
// {30,75,40,25} and conditional 2 -> frame {50,40,2500,2500}, then branch content from y 65:
//   constant 2 {75,90,25,25}, binary 1 {50,165,35,35}
TEST(GraphLayout, FixtureConditionalLaysOutItsThenBranch) {
  const testutil::Loaded l = loadFixture("read/conditional_with_body.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);
  const Box* inThen = hitAt(boxes, Vec2{80, 95});  // constant 2, clear of its grips
  const Box* emptySpace = hitAt(boxes, Vec2{2000, 2000});

  ASSERT_NE(inThen, nullptr);
  ASSERT_NE(emptySpace, nullptr);
  EXPECT_EQ(inThen->path, (FullID{1, 2, THEN_BRANCH_ID, 2}));
  EXPECT_EQ(emptySpace->path, (FullID{1, 2, THEN_BRANCH_ID}));
}

// The fixture reuses id 1 for a node in the function body and one in the then branch:
// only the path tells them apart.
TEST(GraphLayout, FixtureIdOneResolvesPerBlock) {
  const testutil::Loaded l = loadFixture("read/conditional_with_body.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);
  const Box* inFunction = hitAt(boxes, Vec2{35, 80});
  const Box* inThen = hitAt(boxes, Vec2{60, 175});

  ASSERT_NE(inFunction, nullptr);
  ASSERT_NE(inThen, nullptr);
  EXPECT_EQ(inFunction->path, (FullID{1, 1}));
  EXPECT_EQ(inThen->path, (FullID{1, 2, THEN_BRANCH_ID, 1}));
  expectRectNear(inFunction->world, Rect{30, 75, 40, 25});
  expectRectNear(inThen->world, Rect{50, 165, 35, 35});
}

// conditional_empty_scopes.fl: conditional 2 at units (10,3) 100 wide and 100 tall.
//   frame {50,40,500,500}   then branch {50,65,500,475}
TEST(GraphLayout, AnEmptyBranchStillSpansItsConditionalBody) {
  const testutil::Loaded l = loadFixture("read/conditional_empty_scopes.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const std::vector<Box> boxes = layoutGraph(*l.result.tree, kCtx.layout);
  const Box* inThen = hitAt(boxes, Vec2{300, 200});
  const Box* belowIt = hitAt(boxes, Vec2{300, 560});

  ASSERT_NE(inThen, nullptr);
  ASSERT_NE(belowIt, nullptr);
  EXPECT_EQ(inThen->path, (FullID{1, 2, THEN_BRANCH_ID}));
  EXPECT_EQ(belowIt->path, (FullID{1}));  // the conditional ends where its frame does
}
