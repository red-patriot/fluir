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
  constexpr Vec2 kBinaryGrip{137, 97};       // centre of {130,90,15,15}
  constexpr Vec2 kBinaryBar{147, 100};       // inside {145,85,5,25}
  constexpr Vec2 kBinaryBody{127, 107};      // off both grips
  constexpr Vec2 kHeaderGrip{537.5, 62.5};   // centre of {530,55,15,15}
  constexpr Vec2 kCornerGrip{542.5, 542.5};  // centre of {535,535,15,15}

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

TEST(DragTool, TheResizeBarGrowsWidthOnly) {
  Harness h;
  ASSERT_TRUE(h.send(down(kBinaryBar)));

  h.send(move(kBinaryBar + Vec2{15, 20}));
  h.send(up(kBinaryBar + Vec2{15, 20}));

  EXPECT_EQ(h.loc(kBinary).width, 8);
  EXPECT_EQ(h.loc(kBinary).height, 5);
  EXPECT_EQ(h.loc(kBinary).x, 15) << "a resize never moves";
}

TEST(DragTool, ANodeResizeClampsToTheMinimumWidth) {
  Harness h;
  ASSERT_TRUE(h.send(down(kBinaryBar)));

  h.send(move(kBinaryBar + Vec2{-100, 0}));

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
