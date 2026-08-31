#include "../include/editor/core/viewport.hpp"

#include <cmath>

#include <gtest/gtest.h>

namespace {

  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;

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

}  // namespace
