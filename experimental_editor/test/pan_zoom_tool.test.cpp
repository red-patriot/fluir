#include "editor/tools/pan_zoom_tool.hpp"

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "tool_harness.hpp"

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::InputEvent;
  using fluir::editor::PanZoomTool;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::move;
  using testutil::send;
  using testutil::up;

  const EditorContext kCtx;

  InputEvent wheel(double y) { return {.type = InputEvent::Type::Wheel, .pos = {0, 0}, .wheel = {0, y}}; }

}  // namespace

TEST(PanZoomTool, MiddleDragPansAndReleases) {
  EditorState state{kCtx};
  PanZoomTool uut;

  ASSERT_TRUE(send(uut, state, down(Vec2{10, 10}, InputEvent::Button::Middle)));
  EXPECT_TRUE(uut.capturing());
  ASSERT_TRUE(send(uut, state, move(Vec2{15, 25})));
  EXPECT_EQ(state.view.pan, (Vec2{5, 15}));

  ASSERT_TRUE(send(uut, state, up(Vec2{15, 25}, InputEvent::Button::Middle)));
  EXPECT_FALSE(uut.capturing());
  EXPECT_FALSE(send(uut, state, move(Vec2{100, 100})));
  EXPECT_EQ(state.view.pan, (Vec2{5, 15}));
}

TEST(PanZoomTool, LeftPressIsNotAPan) {
  EditorState state{kCtx};
  PanZoomTool uut;

  EXPECT_FALSE(send(uut, state, down(Vec2{10, 10})));
  EXPECT_FALSE(send(uut, state, move(Vec2{30, 30})));
  EXPECT_EQ(state.view.pan, (Vec2{0, 0}));
}

TEST(PanZoomTool, WheelZoomsAndClampsToTheContextRange) {
  EditorState state{kCtx};
  PanZoomTool uut;

  ASSERT_TRUE(send(uut, state, wheel(1)));
  EXPECT_GT(state.view.scale, 1.0);

  for (int i = 0; i < 100; ++i) {
    send(uut, state, wheel(1));
  }
  EXPECT_NEAR(state.view.scale, kCtx.zoom.max, 1e-9) << "a step past the limit lands on it";
  for (int i = 0; i < 200; ++i) {
    send(uut, state, wheel(-1));
  }
  EXPECT_NEAR(state.view.scale, kCtx.zoom.min, 1e-9);
}

// A view left outside the range must still be steerable back into it.
TEST(PanZoomTool, WheelPullsAnOutOfRangeScaleBackToTheLimit) {
  EditorState state{kCtx};
  state.view.scale = 5.0;
  PanZoomTool uut;

  ASSERT_TRUE(send(uut, state, wheel(-1)));

  EXPECT_NEAR(state.view.scale, kCtx.zoom.max, 1e-9);
}

TEST(PanZoomTool, CancelEndsAPan) {
  EditorState state{kCtx};
  PanZoomTool uut;
  ASSERT_TRUE(send(uut, state, down(Vec2{10, 10}, InputEvent::Button::Middle)));

  uut.cancel(state);

  EXPECT_FALSE(uut.capturing());
  EXPECT_FALSE(send(uut, state, move(Vec2{30, 30})));
}
