#include "editor/components/button_actor.hpp"

#include <string>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"
#include "recording_renderer.hpp"

namespace {

  using fluir::editor::ButtonActor;
  using fluir::editor::EditorContext;
  using fluir::editor::Rect;
  using fluir::editor::Subview;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::hasFill;
  using testutil::hasRect;
  using testutil::hasTextAt;
  using testutil::RecordingRenderer;

}  // namespace

TEST(ButtonActor, OnClickInvokesActionRegardlessOfPosition) {
  int clicks = 0;
  ButtonActor actor("Run", [&clicks] { ++clicks; }, Rect{0, 0, 10, 10});

  actor.onClick(Vec2{999, -123});

  EXPECT_EQ(clicks, 1);
}

TEST(ButtonActor, DrawFillsBordersAndLabelsAtBounds) {
  const Rect bounds{0, 0, 25, 25};
  ButtonActor actor("Run", [] {}, bounds);

  const EditorContext ctx;
  RecordingRenderer renderer;
  const Viewport viewport;
  {
    const Subview body{viewport, Rect{0, 0, 1000, 1000}, renderer};
    actor.draw(body, ctx);
  }

  EXPECT_TRUE(hasFill(renderer.calls, bounds));
  EXPECT_TRUE(hasRect(renderer.calls, bounds));
  EXPECT_TRUE(hasTextAt(renderer.calls, "Run", Vec2{bounds.x + ctx.layout.textPad, bounds.y + ctx.layout.textPad}));
}
