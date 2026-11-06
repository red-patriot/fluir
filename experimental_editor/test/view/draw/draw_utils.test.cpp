#include "editor/view/draw/draw_utils.hpp"

#include <gtest/gtest.h>

namespace {

  using fluir::editor::fitInto;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;

  void expectRectNear(const Rect& got, const Rect& want, double tol = 1e-6) {
    EXPECT_NEAR(got.x, want.x, tol);
    EXPECT_NEAR(got.y, want.y, tol);
    EXPECT_NEAR(got.w, want.w, tol);
    EXPECT_NEAR(got.h, want.h, tol);
  }

  TEST(Grips, MoveGripSitsInsideTheTopRightCorner) {
    const Rect frame{10, 20, 100, 50};
    const Rect grip = fluir::editor::moveGrip(frame, 5.0);
    EXPECT_TRUE(frame.contains(grip.topLeft()));
    EXPECT_LE(grip.x + grip.w, frame.x + frame.w);
    EXPECT_LT(grip.y + grip.h, frame.y + frame.h / 2);
    EXPECT_GT(grip.x, frame.x + frame.w / 2);
  }

  TEST(Grips, ResizeCornerHugsTheBottomRightCorner) {
    const Rect frame{10, 20, 100, 50};
    const Rect corner = fluir::editor::resizeCorner(frame, 5.0);
    EXPECT_DOUBLE_EQ(corner.x + corner.w, frame.x + frame.w);
    EXPECT_DOUBLE_EQ(corner.y + corner.h, frame.y + frame.h);
    EXPECT_GT(corner.w, 0.0);
  }

  TEST(FitInto, WiderThanTargetPillarboxesVertically) {
    // 2:1 into a square: full width, half height, centred.
    expectRectNear(fitInto(Rect{10, 20, 100, 100}, Vec2{200, 100}), Rect{10, 45, 100, 50});
  }

  TEST(FitInto, TallerThanTargetPillarboxesHorizontally) {
    expectRectNear(fitInto(Rect{10, 20, 100, 100}, Vec2{100, 200}), Rect{35, 20, 50, 100});
  }

  TEST(FitInto, MatchingAspectFillsTheTargetExactly) {
    expectRectNear(fitInto(Rect{10, 20, 80, 40}, Vec2{16, 8}), Rect{10, 20, 80, 40});
  }

  TEST(FitInto, DegenerateInputCollapsesToTheTargetCentre) {
    for (const Vec2 intrinsic : {Vec2{0, 16}, Vec2{16, 0}, Vec2{-4, -4}}) {
      expectRectNear(fitInto(Rect{10, 20, 100, 60}, intrinsic), Rect{60, 50, 0, 0});
    }
    expectRectNear(fitInto(Rect{10, 20, 0, 60}, Vec2{16, 16}), Rect{10, 50, 0, 0});
  }

}  // namespace
