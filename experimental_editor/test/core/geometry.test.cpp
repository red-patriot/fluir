#include "editor/core/geometry.hpp"

#include <optional>

#include <gtest/gtest.h>

namespace {

  using fluir::editor::intersect;
  using fluir::editor::Rect;

  TEST(GeometryIntersect, PartialOverlapYieldsSharedRegion) {
    const Rect a{.x = 0, .y = 0, .w = 10, .h = 10};
    const Rect b{.x = 5, .y = 5, .w = 10, .h = 10};

    const std::optional<Rect> result = intersect(a, b);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, (Rect{.x = 5, .y = 5, .w = 5, .h = 5}));
  }

  TEST(GeometryIntersect, FullContainmentYieldsInnerRect) {
    const Rect outer{.x = 0, .y = 0, .w = 20, .h = 20};
    const Rect inner{.x = 4, .y = 6, .w = 5, .h = 3};

    const std::optional<Rect> result = intersect(outer, inner);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, inner);
  }

  TEST(GeometryIntersect, EdgeTouchingHasZeroAreaAndIsEmpty) {
    const Rect a{.x = 0, .y = 0, .w = 10, .h = 10};
    const Rect rightEdge{.x = 10, .y = 0, .w = 5, .h = 10};
    const Rect bottomEdge{.x = 0, .y = 10, .w = 10, .h = 5};

    EXPECT_FALSE(intersect(a, rightEdge).has_value());
    EXPECT_FALSE(intersect(a, bottomEdge).has_value());
  }

  TEST(GeometryIntersect, DisjointRectsHaveNoIntersection) {
    const Rect a{.x = 0, .y = 0, .w = 10, .h = 10};
    const Rect b{.x = 50, .y = 50, .w = 10, .h = 10};

    EXPECT_FALSE(intersect(a, b).has_value());
  }

  TEST(GeometryIntersect, IsCommutative) {
    const Rect a{.x = -3, .y = 2, .w = 10, .h = 4};
    const Rect b{.x = 1, .y = -1, .w = 3, .h = 20};
    const Rect far{.x = 100, .y = 100, .w = 1, .h = 1};

    EXPECT_EQ(intersect(a, b), intersect(b, a));
    EXPECT_EQ(intersect(a, far), intersect(far, a));
  }

  TEST(GeometryIntersect, EmptyInputHasNoIntersection) {
    const Rect a{.x = 0, .y = 0, .w = 10, .h = 10};
    const Rect degenerate{.x = 2, .y = 2, .w = 0, .h = 5};

    EXPECT_FALSE(intersect(a, degenerate).has_value());
  }

}  // namespace
