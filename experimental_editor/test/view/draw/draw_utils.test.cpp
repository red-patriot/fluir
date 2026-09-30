#include "editor/view/draw/draw_utils.hpp"

#include <array>

#include <gtest/gtest.h>

#include "editor/core/viewport.hpp"
#include "recording_renderer.hpp"

namespace {

  using fluir::editor::fitInto;
  using fluir::editor::Rect;
  using fluir::editor::Subview;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::expectRectNear;
  using testutil::RecordingRenderer;

  // Any bytes will do: an icon is identified downstream by its span's address.
  constexpr std::array<unsigned char, 4> kSvgBytes{'a', 'b', 'c', 'd'};

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

  TEST(DrawImage, MapsThroughTheViewThenFitsInScreenSpace) {
    RecordingRenderer r;
    const Viewport vp{.pan = {5, 7}, .scale = 2.0};
    const Subview view{vp, Rect{0, 0, 800, 600}, r};

    // World {10,20,40,20} -> screen {25,47,80,40}; the 16x16 fake icon fits to 40x40 centred.
    fluir::editor::draw::drawImage(kSvgBytes, Rect{10, 20, 40, 20}, view, {});

    EXPECT_TRUE(testutil::hasIcon(r.calls, kSvgBytes, Rect{45, 47, 40, 40}));
  }

  TEST(DrawImage, ScalesWithTheView) {
    RecordingRenderer r;
    const Subview view{Viewport{.pan = {}, .scale = 0.5}, Rect{0, 0, 800, 600}, r};

    fluir::editor::draw::drawImage(kSvgBytes, Rect{0, 0, 40, 40}, view, {});

    EXPECT_TRUE(testutil::hasIcon(r.calls, kSvgBytes, Rect{0, 0, 20, 20}));
  }

}  // namespace
