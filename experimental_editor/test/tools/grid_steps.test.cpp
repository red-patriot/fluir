#include "editor/tools/grid_steps.hpp"

#include <gtest/gtest.h>

namespace {

  using fluir::editor::GridSteps;
  using fluir::editor::Vec2;
  using fluir::editor::Vec2i;

  constexpr double UNIT = 5;

}  // namespace

TEST(GridSteps, WholeUnitsStepAtOnce) {
  GridSteps steps;
  steps.start(Vec2{100, 100});

  EXPECT_EQ(steps.advance(Vec2{110, 85}, UNIT), (Vec2i{2, -3}));
}

TEST(GridSteps, SubUnitMotionCarriesOver) {
  GridSteps steps;
  steps.start(Vec2{100, 100});

  EXPECT_EQ(steps.advance(Vec2{103, 100}, UNIT), (Vec2i{0, 0}));
  EXPECT_EQ(steps.advance(Vec2{106, 100}, UNIT), (Vec2i{1, 0}));
  EXPECT_EQ(steps.advance(Vec2{109, 100}, UNIT), (Vec2i{0, 0}));
  EXPECT_EQ(steps.advance(Vec2{110, 100}, UNIT), (Vec2i{1, 0}));
}

TEST(GridSteps, StartForgetsTheLastGesturesRemainder) {
  GridSteps steps;
  steps.start(Vec2{100, 100});
  steps.advance(Vec2{104, 100}, UNIT);

  steps.start(Vec2{0, 0});

  EXPECT_EQ(steps.advance(Vec2{4, 0}, UNIT), (Vec2i{0, 0}));
}
