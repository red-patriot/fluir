#include "editor/core/viewport.hpp"

#include <cmath>

#include <gtest/gtest.h>

#include "editor/core/renderer.hpp"
#include "recording_renderer.hpp"

namespace {

  using fluir::editor::Rect;
  using fluir::editor::Subview;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::clipsCovering;
  using testutil::DrawCall;
  using testutil::opsOf;
  using testutil::RecordingRenderer;

  // Scale limits wide enough not to bite: these cases are about the ratio, not the clamp.
  constexpr Vec2 kAnyScale{0.001, 1000.0};

  TEST(Viewport, WorldToScreenAppliesScaleThenPan) {
    const Viewport vp{.pan = {10.0, 20.0}, .scale = 2.0};
    const Vec2 s = vp.worldToScreen({3.0, 4.0});
    EXPECT_NEAR(s.x, 16.0, 1e-4);
    EXPECT_NEAR(s.y, 28.0, 1e-4);
  }

  TEST(Viewport, RectToScreenMapsTopLeftAndScalesSize) {
    const Viewport vp{.pan = {10.0, 20.0}, .scale = 2.0};
    const Rect r = vp.toScreen(Rect{3.0, 4.0, 5.0, 6.0});
    EXPECT_NEAR(r.x, 16.0, 1e-6);
    EXPECT_NEAR(r.y, 28.0, 1e-6);
    EXPECT_NEAR(r.w, 10.0, 1e-6);
    EXPECT_NEAR(r.h, 12.0, 1e-6);
  }

  TEST(Viewport, ScreenToWorldInvertsWorldToScreen) {
    const Viewport vp{.pan = {-7.5, 12.0}, .scale = 3.0};
    const Vec2 w = vp.screenToWorld(vp.worldToScreen({42.0, -13.0}));
    EXPECT_NEAR(w.x, 42.0, 1e-4);
    EXPECT_NEAR(w.y, -13.0, 1e-4);
  }

  TEST(Viewport, ZoomAboutKeepsPivotWorldPointFixed) {
    Viewport vp{.pan = {5.0, 5.0}, .scale = 1.0};
    const Vec2 pivot{100.0, 80.0};
    const Vec2 before = vp.screenToWorld(pivot);
    vp.zoomAbout(pivot, 1.5);
    const Vec2 after = vp.screenToWorld(pivot);
    EXPECT_NEAR(after.x, before.x, 1e-4);
    EXPECT_NEAR(after.y, before.y, 1e-4);
    EXPECT_NEAR(vp.scale, 1.5, 1e-4);
  }

  TEST(Viewport, ZoomAboutComposes) {
    Viewport vp;
    const Vec2 pivot{50.0, 50.0};
    vp.zoomAbout(pivot, 2.0);
    vp.zoomAbout(pivot, 2.0);
    EXPECT_NEAR(vp.scale, 4.0, 1e-4);
  }

  TEST(Viewport, FitRectScalesToMinRatioAndCentres) {
    Viewport vp;
    const Rect rect{0.0, 0.0, 100.0, 50.0};
    vp.fitRect(rect, {200.0, 200.0}, kAnyScale.x, kAnyScale.y);
    EXPECT_NEAR(vp.scale, 2.0, 1e-4);
    const Vec2 c = vp.worldToScreen(rect.center());
    EXPECT_NEAR(c.x, 100.0, 1e-4);
    EXPECT_NEAR(c.y, 100.0, 1e-4);
  }

  TEST(Viewport, FitRectHandlesNonOriginRect) {
    Viewport vp;
    const Rect rect{40.0, -20.0, 20.0, 20.0};
    vp.fitRect(rect, {100.0, 100.0}, kAnyScale.x, kAnyScale.y);
    EXPECT_NEAR(vp.scale, 5.0, 1e-4);
    const Vec2 c = vp.worldToScreen(rect.center());
    EXPECT_NEAR(c.x, 50.0, 1e-4);
    EXPECT_NEAR(c.y, 50.0, 1e-4);
  }

  // A small graph fits at a scale no wheel step could reach: fitRect must clamp
  // into the same range the zoom does, or zooming is dead on arrival.
  TEST(Viewport, FitRectClampsScaleToTheMaxAndStillCentres) {
    Viewport vp;
    const Rect rect{-60.0, -115.0, 200.0, 150.0};
    vp.fitRect(rect, {1280.0, 800.0}, 0.25, 2.5);
    EXPECT_NEAR(vp.scale, 2.5, 1e-6);
    const Vec2 c = vp.worldToScreen(rect.center());
    EXPECT_NEAR(c.x, 640.0, 1e-6);
    EXPECT_NEAR(c.y, 400.0, 1e-6);
  }

  TEST(Viewport, FitRectClampsScaleToTheMinAndStillCentres) {
    Viewport vp;
    const Rect rect{0.0, 0.0, 100000.0, 100000.0};
    vp.fitRect(rect, {1280.0, 800.0}, 0.25, 2.5);
    EXPECT_NEAR(vp.scale, 0.25, 1e-6);
    const Vec2 c = vp.worldToScreen(rect.center());
    EXPECT_NEAR(c.x, 640.0, 1e-6);
    EXPECT_NEAR(c.y, 400.0, 1e-6);
  }

  TEST(Viewport, FitRectGuardsZeroSize) {
    Viewport vp;
    vp.fitRect({0.0, 0.0, 0.0, 0.0}, {100.0, 100.0}, kAnyScale.x, kAnyScale.y);
    EXPECT_TRUE(std::isfinite(vp.scale));
    const Vec2 s = vp.worldToScreen({0.0, 0.0});
    EXPECT_TRUE(std::isfinite(s.x));
    EXPECT_TRUE(std::isfinite(s.y));
  }

  TEST(Geometry, Vec2Arithmetic) {
    const Vec2 a{3.0, 4.0};
    const Vec2 b{1.0, 2.0};
    EXPECT_EQ(a + b, (Vec2{4.0, 6.0}));
    EXPECT_EQ(a - b, (Vec2{2.0, 2.0}));
    EXPECT_EQ(a * 2.0, (Vec2{6.0, 8.0}));
    EXPECT_EQ(a / 2.0, (Vec2{1.5, 2.0}));
    EXPECT_TRUE(a == (Vec2{3.0, 4.0}));
    EXPECT_FALSE(a == b);
  }

  TEST(Geometry, RectAnchors) {
    const Rect r{10.0, 20.0, 40.0, 60.0};
    EXPECT_EQ(r.topLeft(), (Vec2{10.0, 20.0}));
    EXPECT_EQ(r.center(), (Vec2{30.0, 50.0}));
  }

  // Any frame; these cases only exercise the transform, not the clip rect. The
  // frame corners at (0,0) so its top-left origin is unchanged by the fold.
  constexpr Rect kAnyFrame{0.0, 0.0, 1000.0, 1000.0};

  TEST(Subview, RootKeepsTheViewportTransformUnchanged) {
    RecordingRenderer out;
    // The root rect is a screen clip, never a world origin: the transform is the raw viewport.
    for (const Viewport vp :
         {Viewport{}, Viewport{.pan = {100.0, 50.0}, .scale = 2.0}, Viewport{.pan = {-13.0, 640.0}, .scale = 0.25}}) {
      const Subview view{vp, Rect{30.0, 40.0, 1000.0, 1000.0}, out};
      const Vec2 want = vp.worldToScreen(Vec2{5.0, 7.0});
      EXPECT_NEAR(view.toScreen(Vec2{5.0, 7.0}).x, want.x, 1e-6);
      EXPECT_NEAR(view.toScreen(Vec2{5.0, 7.0}).y, want.y, 1e-6);
    }
  }

  TEST(Subview, RectToScreenScalesSizeOnly) {
    RecordingRenderer out;
    const Viewport vp{.pan = {10.0, 20.0}, .scale = 3.0};
    const Subview view{vp, kAnyFrame, out};
    const Rect r = view.toScreen(Rect{4.0, 5.0, 6.0, 8.0});
    EXPECT_NEAR(r.x, 4.0 * 3.0 + 10.0, 1e-6);
    EXPECT_NEAR(r.y, 5.0 * 3.0 + 20.0, 1e-6);
    EXPECT_NEAR(r.w, 18.0, 1e-6);
    EXPECT_NEAR(r.h, 24.0, 1e-6);
  }

  TEST(Subview, ChildOriginAddsOnComposedTransform) {
    RecordingRenderer out;
    const Viewport vp{.pan = {100.0, 50.0}, .scale = 2.0};
    const Subview parent{vp, Rect{30.0, 40.0, 1000.0, 1000.0}, out};

    const Subview same = parent.child(kAnyFrame);
    EXPECT_EQ(same.toScreen(Vec2{5.0, 7.0}), parent.toScreen(Vec2{5.0, 7.0}));

    const Subview shifted = parent.child(Rect{10.0, 0.0, 1000.0, 1000.0});
    const Vec2 a = parent.toScreen(Vec2{5.0, 7.0});
    const Vec2 b = shifted.toScreen(Vec2{5.0, 7.0});
    EXPECT_NEAR(b.x - a.x, 10.0 * vp.scale, 1e-6);
    EXPECT_NEAR(b.y - a.y, 0.0, 1e-6);
  }

  TEST(Subview, AlwaysPushesOneClipPerSubviewPoppedInReverse) {
    RecordingRenderer out;
    {
      const Viewport vp;
      const Subview view{vp, Rect{0.0, 0.0, 10.0, 10.0}, out};
      EXPECT_EQ(opsOf(out.calls, DrawCall::Op::PushClip).size(), 1u);
      EXPECT_EQ(opsOf(out.calls, DrawCall::Op::PopClip).size(), 0u);
      {
        const Subview nested = view.child(Rect{0.0, 0.0, 5.0, 5.0});
        EXPECT_EQ(opsOf(out.calls, DrawCall::Op::PushClip).size(), 2u);
        EXPECT_EQ(opsOf(out.calls, DrawCall::Op::PopClip).size(), 0u);
      }
      EXPECT_EQ(opsOf(out.calls, DrawCall::Op::PopClip).size(), 1u);  // nested popped at its scope exit
    }
    EXPECT_EQ(opsOf(out.calls, DrawCall::Op::PushClip).size(), 2u);
    EXPECT_EQ(opsOf(out.calls, DrawCall::Op::PopClip).size(), 2u);  // root popped last

    // LIFO order: outer pushed first, inner pushed second, both popped after
    // (root's pop closes the sequence last).
    const std::vector<DrawCall> want{
      {DrawCall::Op::PushClip, Rect{0.0, 0.0, 10.0, 10.0}, {}, {}, {}},
      {DrawCall::Op::PushClip, Rect{0.0, 0.0, 5.0, 5.0}, {}, {}, {}},
      {DrawCall::Op::PopClip, {}, {}, {}, {}},
      {DrawCall::Op::PopClip, {}, {}, {}, {}},
    };
    EXPECT_EQ(out.calls, want);
  }

  TEST(Subview, RootClipIsTheScreenRectAsGiven) {
    RecordingRenderer out;
    const Viewport vp{.pan = {100.0, 50.0}, .scale = 2.0};
    const Subview view{vp, Rect{30.0, 40.0, 10.0, 20.0}, out};
    EXPECT_EQ(clipsCovering(out.calls, Rect{30.0, 40.0, 10.0, 20.0}).size(), 1u);
  }

  TEST(Subview, NestedClipRectIsBoundsInScreenSpace) {
    RecordingRenderer out;
    const Viewport vp{.pan = {100.0, 50.0}, .scale = 2.0};
    const Subview root{vp, kAnyFrame, out};
    const Subview view = root.child(Rect{30.0, 40.0, 10.0, 20.0});
    // frame top-left (30,40) world -> (30*2+100, 40*2+50); size scaled by 2.
    EXPECT_EQ(clipsCovering(out.calls, Rect{160.0, 130.0, 20.0, 40.0}).size(), 1u);
  }

  // The blank-program case: fitRect on empty bounds leaves pan at half the output size.
  TEST(Subview, RootClipCoversTheWholeOutputWhenPanIsLarge) {
    RecordingRenderer out;
    const Viewport vp{.pan = {640.0, 400.0}, .scale = 1.0};
    const Rect output{0.0, 0.0, 1280.0, 800.0};
    const Subview view{vp, output, out};
    ASSERT_EQ(clipsCovering(out.calls, output).size(), 1u);
    const Vec2 topLeftish = view.toScreen(vp.screenToWorld(Vec2{10.0, 10.0}));
    EXPECT_TRUE(output.contains(topLeftish));
  }

}  // namespace
