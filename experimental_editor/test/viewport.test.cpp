#include "../include/editor/core/viewport.hpp"

#include <cmath>
#include <string_view>

#include <gtest/gtest.h>

#include "../include/editor/core/renderer.hpp"

namespace {

  using fluir::editor::Rect;
  using fluir::editor::Renderer;
  using fluir::editor::Subview;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;

  // Minimal Renderer that only tracks clip-stack calls.
  struct ClipCountingRenderer : Renderer {
    int pushes = 0;
    int pops = 0;
    Rect lastClip{};

    void beginFrame() override { }
    void endFrame() override { }
    void drawRect(Rect) override { }
    void fillRect(Rect) override { }
    void drawLine(Vec2, Vec2) override { }
    void drawText(Vec2, std::string_view) override { }
    void pushClip(Rect r) override {
      ++pushes;
      lastClip = r;
    }
    void popClip() override { ++pops; }
  };

  TEST(Viewport, WorldToScreenAppliesScaleThenPan) {
    const Viewport vp{.pan = {10.0, 20.0}, .scale = 2.0};
    const Vec2 s = vp.worldToScreen({3.0, 4.0});
    EXPECT_NEAR(s.x, 16.0, 1e-4);
    EXPECT_NEAR(s.y, 28.0, 1e-4);
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
    vp.fitRect(rect, {200.0, 200.0});
    EXPECT_NEAR(vp.scale, 2.0, 1e-4);
    const Vec2 c = vp.worldToScreen(rect.center());
    EXPECT_NEAR(c.x, 100.0, 1e-4);
    EXPECT_NEAR(c.y, 100.0, 1e-4);
  }

  TEST(Viewport, FitRectHandlesNonOriginRect) {
    Viewport vp;
    const Rect rect{40.0, -20.0, 20.0, 20.0};
    vp.fitRect(rect, {100.0, 100.0});
    EXPECT_NEAR(vp.scale, 5.0, 1e-4);
    const Vec2 c = vp.worldToScreen(rect.center());
    EXPECT_NEAR(c.x, 50.0, 1e-4);
    EXPECT_NEAR(c.y, 50.0, 1e-4);
  }

  TEST(Viewport, FitRectGuardsZeroSize) {
    Viewport vp;
    vp.fitRect({0.0, 0.0, 0.0, 0.0}, {100.0, 100.0});
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

  // Any bounds; these cases only exercise the transform, not the clip rect.
  constexpr Rect kAnyBounds{0.0, 0.0, 1000.0, 1000.0};

  TEST(Subview, RootComposesViewportWithOrigin) {
    ClipCountingRenderer out;
    {
      const Viewport vp;
      const Subview view{vp, Vec2{30.0, 40.0}, kAnyBounds, out};
      const Vec2 s = view.toScreen(Vec2{5.0, 7.0});
      EXPECT_NEAR(s.x, 35.0, 1e-6);
      EXPECT_NEAR(s.y, 47.0, 1e-6);
    }
    {
      const Viewport vp{.pan = {100.0, 50.0}, .scale = 2.0};
      const Subview view{vp, Vec2{30.0, 40.0}, kAnyBounds, out};
      // (local + origin) * scale + pan
      const Vec2 s = view.toScreen(Vec2{5.0, 7.0});
      EXPECT_NEAR(s.x, (5.0 + 30.0) * 2.0 + 100.0, 1e-6);
      EXPECT_NEAR(s.y, (7.0 + 40.0) * 2.0 + 50.0, 1e-6);
    }
  }

  TEST(Subview, RectToScreenScalesSizeOnly) {
    ClipCountingRenderer out;
    const Viewport vp{.pan = {10.0, 20.0}, .scale = 3.0};
    const Subview view{vp, Vec2{0.0, 0.0}, kAnyBounds, out};
    const Rect r = view.toScreen(Rect{4.0, 5.0, 6.0, 8.0});
    EXPECT_NEAR(r.x, 4.0 * 3.0 + 10.0, 1e-6);
    EXPECT_NEAR(r.y, 5.0 * 3.0 + 20.0, 1e-6);
    EXPECT_NEAR(r.w, 18.0, 1e-6);
    EXPECT_NEAR(r.h, 24.0, 1e-6);
  }

  TEST(Subview, ChildOriginAddsOnComposedTransform) {
    ClipCountingRenderer out;
    const Viewport vp{.pan = {100.0, 50.0}, .scale = 2.0};
    const Subview parent{vp, Vec2{30.0, 40.0}, kAnyBounds, out};

    const Subview same = parent.child(Vec2{0.0, 0.0}, kAnyBounds);
    EXPECT_EQ(same.toScreen(Vec2{5.0, 7.0}), parent.toScreen(Vec2{5.0, 7.0}));

    const Subview shifted = parent.child(Vec2{10.0, 0.0}, kAnyBounds);
    const Vec2 a = parent.toScreen(Vec2{5.0, 7.0});
    const Vec2 b = shifted.toScreen(Vec2{5.0, 7.0});
    EXPECT_NEAR(b.x - a.x, 10.0 * vp.scale, 1e-6);
    EXPECT_NEAR(b.y - a.y, 0.0, 1e-6);
  }

  TEST(Subview, AlwaysPushesOneClipPerSubviewPoppedInReverse) {
    ClipCountingRenderer out;
    {
      const Viewport vp;
      const Subview view{vp, Vec2{0.0, 0.0}, Rect{0.0, 0.0, 10.0, 10.0}, out};
      EXPECT_EQ(out.pushes, 1);
      EXPECT_EQ(out.pops, 0);
      {
        const Subview nested = view.child(Vec2{0.0, 0.0}, Rect{0.0, 0.0, 5.0, 5.0});
        EXPECT_EQ(out.pushes, 2);
        EXPECT_EQ(out.pops, 0);
      }
      EXPECT_EQ(out.pops, 1);  // nested popped at its scope exit
    }
    EXPECT_EQ(out.pushes, 2);
    EXPECT_EQ(out.pops, 2);  // root popped last
  }

  TEST(Subview, ClipRectIsBoundsInScreenSpace) {
    ClipCountingRenderer out;
    const Viewport vp{.pan = {100.0, 50.0}, .scale = 2.0};
    const Subview view{vp, Vec2{30.0, 40.0}, Rect{0.0, 0.0, 10.0, 20.0}, out};
    EXPECT_EQ(out.pushes, 1);
    // bounds top-left (0,0) local -> ((0+30)*2+100, (0+40)*2+50); size scaled by 2.
    EXPECT_NEAR(out.lastClip.x, 160.0, 1e-6);
    EXPECT_NEAR(out.lastClip.y, 130.0, 1e-6);
    EXPECT_NEAR(out.lastClip.w, 20.0, 1e-6);
    EXPECT_NEAR(out.lastClip.h, 40.0, 1e-6);
  }

}  // namespace
