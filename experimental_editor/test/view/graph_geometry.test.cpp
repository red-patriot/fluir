#include "editor/view/graph_geometry.hpp"

#include <gtest/gtest.h>

#include "compiler/models/location.hpp"
#include "editor/core/geometry.hpp"

namespace {

  using fluir::FlowGraphLocation;
  using fluir::editor::atOrigin;
  using fluir::editor::bodyOrigin;
  using fluir::editor::dotRect;
  using fluir::editor::localRect;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;

}  // namespace

TEST(GraphGeometry, LocalRectScalesLocationByUnitPx) {
  // int_constants.fl constant id=1: x=2 y=20 w=5 h=5, unitPx=5.
  const FlowGraphLocation loc{.x = 2, .y = 20, .z = 1, .width = 5, .height = 5};
  EXPECT_EQ(localRect(loc, 5.0), (Rect{10, 100, 25, 25}));
}

TEST(GraphGeometry, BodyOriginOffsetsByHeaderHeight) {
  const Vec2 origin{50, 50};
  EXPECT_EQ(bodyOrigin(origin, 25.0), (Vec2{50, 75}));
}

TEST(GraphGeometry, AtOriginShiftsLocalRectKeepingSize) {
  // Constant id=1's world rect, as the scene places it.
  const Vec2 body{50, 75};
  const Rect local{10, 100, 25, 25};
  EXPECT_EQ(atOrigin(body, local), (Rect{60, 175, 25, 25}));
}

TEST(GraphGeometry, DotRectIsCenteredOnAnchor) { EXPECT_EQ(dotRect(Vec2{122, 84.5}, 6.0), (Rect{119, 81.5, 6, 6})); }
