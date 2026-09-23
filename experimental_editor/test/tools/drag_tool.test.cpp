#include "editor/tools/drag_tool.hpp"

#include <memory>

#include <gtest/gtest.h>

#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/delete.hpp"
#include "editor/view/graph_layout.hpp"
#include "recording_renderer.hpp"
#include "tool_harness.hpp"

// simple_binary_expr.fl: function 1 at units (10,10) 100x100 -> {50,50,500,500};
// binary 1 at body units (15,2) 5x5 -> {125,85,25,25}. unitPx 5: 5 px per unit.
// The tree moves live; one edit is recorded per gesture.

namespace {

  using fluir::FlowGraphLocation;
  using fluir::FullID;
  using fluir::editor::DeleteTransaction;
  using fluir::editor::DragTool;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::InputEvent;
  using fluir::editor::locationAt;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::key;
  using testutil::move;
  using testutil::send;
  using testutil::up;

  const EditorContext kCtx;
  const FullID kBinary{1, 1};
  const FullID kFunction{1};
  constexpr Vec2 kBinaryGrip{137, 97};        // centre of {130,90,15,15}
  constexpr Vec2 kBinaryRightEdge{147, 100};  // inside the right edge {145,85,5,25}
  constexpr Vec2 kBinaryBody{127, 107};       // off both grips
  constexpr Vec2 kHeaderGrip{537.5, 62.5};    // centre of {530,55,15,15}
  constexpr Vec2 kCornerGrip{542.5, 542.5};   // centre of {535,535,15,15}

  struct Harness {
    EditorState state{kCtx};
    DragTool tool;

    Harness() { testutil::loadInto(state, "read/simple_binary_expr.fl"); }

    bool send(const InputEvent& event) { return testutil::send(tool, state, event); }
    FlowGraphLocation loc(const FullID& path) const { return *locationAt(state.editor.tree(), path); }
  };

}  // namespace

TEST(DragTool, OnlyAPressOnAGripClaimsTheGesture) {
  Harness h;

  EXPECT_FALSE(h.send(down(kBinaryBody)));
  EXPECT_FALSE(h.tool.capturing());
  EXPECT_FALSE(h.send(down(Vec2{-10, -10})));

  EXPECT_TRUE(h.send(down(kBinaryGrip)));
  EXPECT_TRUE(h.tool.capturing());
}

TEST(DragTool, AMoveGripDragSnapsToWholeUnitsAndMovesTheTreeLive) {
  Harness h;
  ASSERT_TRUE(h.send(down(kBinaryGrip)));

  EXPECT_TRUE(h.send(move(kBinaryGrip + Vec2{10, 5})));

  EXPECT_EQ(h.loc(kBinary).x, 17);
  EXPECT_EQ(h.loc(kBinary).y, 3);
  EXPECT_FALSE(h.state.editor.canUndo()) << "nothing is recorded until release";
}

TEST(DragTool, SubUnitMovesAccumulate) {
  Harness h;
  ASSERT_TRUE(h.send(down(kBinaryGrip)));

  h.send(move(kBinaryGrip + Vec2{3, 0}));
  EXPECT_EQ(h.loc(kBinary).x, 15);
  h.send(move(kBinaryGrip + Vec2{6, 0}));
  EXPECT_EQ(h.loc(kBinary).x, 16);
}

TEST(DragTool, ReleaseRecordsOneUndoableEditForTheWholeGesture) {
  Harness h;
  const auto before = h.state.editor.tree();
  ASSERT_TRUE(h.send(down(kBinaryGrip)));
  h.send(move(kBinaryGrip + Vec2{5, 0}));
  h.send(move(kBinaryGrip + Vec2{10, 5}));

  EXPECT_TRUE(h.send(up(kBinaryGrip + Vec2{10, 5})));

  EXPECT_FALSE(h.tool.capturing());
  EXPECT_EQ(h.loc(kBinary).x, 17);
  ASSERT_TRUE(h.state.editor.undo());
  EXPECT_EQ(h.state.editor.tree(), before);
  EXPECT_FALSE(h.state.editor.canUndo());
}

TEST(DragTool, AGestureThatEndsWhereItStartedRecordsNothing) {
  Harness h;
  const auto before = h.state.editor.tree();
  ASSERT_TRUE(h.send(down(kBinaryGrip)));
  h.send(move(kBinaryGrip + Vec2{10, 0}));
  h.send(move(kBinaryGrip));

  h.send(up(kBinaryGrip));

  EXPECT_EQ(h.state.editor.tree(), before);
  EXPECT_FALSE(h.state.editor.canUndo());
}

TEST(DragTool, CancelRestoresTheTreeAndRecordsNothing) {
  Harness h;
  const auto before = h.state.editor.tree();
  ASSERT_TRUE(h.send(down(kBinaryGrip)));
  h.send(move(kBinaryGrip + Vec2{10, 0}));

  h.tool.cancel(h.state);

  EXPECT_FALSE(h.tool.capturing());
  EXPECT_EQ(h.state.editor.tree(), before);
  EXPECT_FALSE(h.state.editor.canUndo());
}

TEST(DragTool, EscapeCancelsALiveGesture) {
  Harness h;
  const auto before = h.state.editor.tree();
  ASSERT_TRUE(h.send(down(kBinaryGrip)));
  h.send(move(kBinaryGrip + Vec2{10, 0}));

  EXPECT_TRUE(h.send(key(InputEvent::Key::Escape)));

  EXPECT_FALSE(h.tool.capturing());
  EXPECT_EQ(h.state.editor.tree(), before);
}

TEST(DragTool, TheRightEdgeGrowsWidthOnly) {
  Harness h;
  ASSERT_TRUE(h.send(down(kBinaryRightEdge)));

  h.send(move(kBinaryRightEdge + Vec2{15, 20}));
  h.send(up(kBinaryRightEdge + Vec2{15, 20}));

  EXPECT_EQ(h.loc(kBinary).width, 8);
  EXPECT_EQ(h.loc(kBinary).height, 5);
  EXPECT_EQ(h.loc(kBinary).x, 15) << "a resize never moves";
}

TEST(DragTool, ANodeResizeClampsToTheMinimumWidth) {
  Harness h;
  ASSERT_TRUE(h.send(down(kBinaryRightEdge)));

  h.send(move(kBinaryRightEdge + Vec2{-100, 0}));

  EXPECT_EQ(h.loc(kBinary).width, 4);
}

TEST(DragTool, TheHeaderGripMovesTheFunction) {
  Harness h;
  ASSERT_TRUE(h.send(down(kHeaderGrip)));

  h.send(move(kHeaderGrip + Vec2{10, 10}));

  EXPECT_EQ(h.loc(kFunction).x, 12);
  EXPECT_EQ(h.loc(kFunction).y, 12);
  EXPECT_EQ(h.loc(kFunction).width, 100) << "a header drag never resizes";
}

TEST(DragTool, BodyNodesFollowAFunctionMove) {
  Harness h;
  ASSERT_TRUE(h.send(down(kHeaderGrip)));

  h.send(move(kHeaderGrip + Vec2{50, 20}));

  const auto boxes = fluir::editor::layoutGraph(h.state.editor.tree(), kCtx.layout);
  const auto* hit = fluir::editor::hitAt(boxes, kBinaryBody + Vec2{50, 20});
  ASSERT_NE(hit, nullptr);
  EXPECT_EQ(hit->path, kBinary);
  testutil::expectRectNear(hit->world, fluir::editor::Rect{175, 105, 25, 25});
}

TEST(DragTool, TheCornerGripResizesTheFunctionInBothAxes) {
  Harness h;
  ASSERT_TRUE(h.send(down(kCornerGrip)));

  h.send(move(kCornerGrip + Vec2{15, 5}));

  EXPECT_EQ(h.loc(kFunction).width, 103);
  EXPECT_EQ(h.loc(kFunction).height, 101);
}

TEST(DragTool, AFunctionResizeClampsToItsOwnMinimum) {
  Harness h;
  ASSERT_TRUE(h.send(down(kCornerGrip)));

  h.send(move(kCornerGrip + Vec2{-1000, -1000}));

  EXPECT_EQ(h.loc(kFunction).width, 15);
  EXPECT_EQ(h.loc(kFunction).height, 15);
}

TEST(DragTool, DragDeltasAreMeasuredInWorldSpace) {
  Harness h;
  h.state.view.scale = 2.0;
  ASSERT_TRUE(h.send(down(kBinaryGrip * 2.0)));

  h.send(move(kBinaryGrip * 2.0 + Vec2{20, 0}));  // 10 world px

  EXPECT_EQ(h.loc(kBinary).x, 17);
}

// top_level_comment_only.fl: comment 1 at units (10,10) 25x25 -> {50,50,125,125}; move grip {155,55,15,15}.
TEST(DragTool, AMoveGripDragMovesATopLevelComment) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/top_level_comment_only.fl");
  const auto before = state.editor.tree();
  DragTool tool;
  const Vec2 grip{162.5, 62.5};

  ASSERT_TRUE(send(tool, state, down(grip)));
  send(tool, state, move(grip + Vec2{10, 5}));
  EXPECT_TRUE(send(tool, state, up(grip + Vec2{10, 5})));

  EXPECT_EQ(locationAt(state.editor.tree(), FullID{1})->x, 12);
  EXPECT_EQ(locationAt(state.editor.tree(), FullID{1})->y, 11);
  ASSERT_TRUE(state.editor.undo());
  EXPECT_EQ(state.editor.tree(), before);
  EXPECT_FALSE(state.editor.canUndo());
}

// Comment corner {160,160,15,15}: one diagonal gesture sizes both axes, as a function's does.
TEST(DragTool, TheCornerGripResizesACommentInBothAxes) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/top_level_comment_only.fl");
  const auto before = state.editor.tree();
  DragTool tool;
  const Vec2 corner{167.5, 167.5};

  ASSERT_TRUE(send(tool, state, down(corner)));
  send(tool, state, move(corner + Vec2{15, 10}));
  EXPECT_TRUE(send(tool, state, up(corner + Vec2{15, 10})));

  const FlowGraphLocation loc = *locationAt(state.editor.tree(), FullID{1});
  EXPECT_EQ(loc.width, 28);
  EXPECT_EQ(loc.height, 27);
  EXPECT_EQ(loc.x, 10);
  EXPECT_EQ(loc.y, 10);
  ASSERT_TRUE(state.editor.undo());
  EXPECT_EQ(state.editor.tree(), before);
}

TEST(DragTool, ACommentResizeClampsToItsMinimum) {
  EditorState state{kCtx};
  testutil::loadInto(state, "read/top_level_comment_only.fl");
  DragTool tool;
  const Vec2 corner{167.5, 167.5};

  ASSERT_TRUE(send(tool, state, down(corner)));
  send(tool, state, move(corner + Vec2{-1000, -1000}));

  EXPECT_EQ(locationAt(state.editor.tree(), FullID{1})->width, 8);
  EXPECT_EQ(locationAt(state.editor.tree(), FullID{1})->height, 8);
}

// Undo or delete under a live gesture must not crash or record a stale edit.
TEST(DragTool, ReleaseAfterThePathVanishedRecordsNothing) {
  Harness h;
  ASSERT_TRUE(h.send(down(kBinaryGrip)));
  h.send(move(kBinaryGrip + Vec2{10, 0}));
  h.state.editor.tree().declarations.clear();

  h.send(up(kBinaryGrip));

  EXPECT_FALSE(h.tool.capturing());
  EXPECT_FALSE(h.state.editor.canUndo());
}

namespace {

  using fluir::editor::THEN_BRANCH_ID;

  // Function 1 {0,0,500,500} holds conditional 20 -> frame {10,35,100,90}, one header band over its
  // then branch, which holds constant 1 at {15,65,50,50}.
  //   conditional move grip {90,40,15,15}   corner grip {95,110,15,15}
  //   nested constant move grip {45,70,15,15}
  fluir::pt::ParseTree conditionalTree() {
    fluir::pt::Block then;
    then.nodes.emplace(1,
                       fluir::pt::Constant{.id = 1,
                                           .location = {.x = 1, .y = 1, .z = 0, .width = 10, .height = 10},
                                           .value = fluir::literals_types::I32{0}});

    fluir::pt::FunctionDecl fn;
    fn.id = 1;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    fn.body.nodes.emplace(20,
                          fluir::pt::Conditional{.id = 20,
                                                 .location = {.x = 2, .y = 2, .z = 0, .width = 20, .height = 18},
                                                 .condition = {},
                                                 .inputs = {},
                                                 .outputs = {},
                                                 .thenScope = xyz::indirect{std::move(then)},
                                                 .elseScope = xyz::indirect<fluir::pt::Block>{}});

    fluir::pt::ParseTree tree;
    tree.declarations.emplace(1, fluir::pt::Declaration{std::move(fn)});
    return tree;
  }

  struct NestedHarness {
    EditorState state{kCtx};
    DragTool tool;

    NestedHarness() { state.editor.load(conditionalTree()); }

    bool send(const InputEvent& event) { return testutil::send(tool, state, event); }
    FlowGraphLocation loc(const FullID& path) const { return *locationAt(state.editor.tree(), path); }
  };

  const FullID kConditional{1, 20};
  const FullID kNested{1, 20, THEN_BRANCH_ID, 1};
  constexpr Vec2 kNestedGrip{52, 77};           // centre of {45,70,15,15}
  constexpr Vec2 kConditionalGrip{97, 47};      // centre of {90,40,15,15}
  constexpr Vec2 kConditionalCorner{102, 117};  // centre of {95,110,15,15}

}  // namespace

TEST(DragTool, ANestedNodeDragsInsideItsBranch) {
  NestedHarness h;
  ASSERT_TRUE(h.send(down(kNestedGrip)));

  EXPECT_TRUE(h.send(move(kNestedGrip + Vec2{10, 5})));

  EXPECT_EQ(h.loc(kNested).x, 3);
  EXPECT_EQ(h.loc(kNested).y, 2);
  EXPECT_TRUE(h.send(up(kNestedGrip + Vec2{10, 5})));
  EXPECT_TRUE(h.state.editor.canUndo());
}

TEST(DragTool, AConditionalMoveGripMovesTheConditional) {
  NestedHarness h;
  ASSERT_TRUE(h.send(down(kConditionalGrip)));

  EXPECT_TRUE(h.send(move(kConditionalGrip + Vec2{10, 10})));

  EXPECT_EQ(h.loc(kConditional).x, 4);
  EXPECT_EQ(h.loc(kConditional).y, 4);
}

// The conditional owns its height now, so its corner grip resizes both axes.
TEST(DragTool, AConditionalCornerGripResizesBothAxes) {
  NestedHarness h;
  ASSERT_TRUE(h.send(down(kConditionalCorner)));

  EXPECT_TRUE(h.send(move(kConditionalCorner + Vec2{20, 25})));

  EXPECT_EQ(h.loc(kConditional).width, 24);
  EXPECT_EQ(h.loc(kConditional).height, 23);
}

TEST(DragTool, AConditionalCannotCollapse) {
  NestedHarness h;
  ASSERT_TRUE(h.send(down(kConditionalCorner)));

  EXPECT_TRUE(h.send(move(kConditionalCorner - Vec2{200, 200})));

  EXPECT_GE(h.loc(kConditional).width, 10);
  EXPECT_GE(h.loc(kConditional).height, 10);
}
