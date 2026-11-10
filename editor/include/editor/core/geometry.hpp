#pragma once

#include <algorithm>
#include <optional>

namespace fluir::editor {
  //? Consider using a library for linalg instead

  /** 2D vector / point. Screen and world space are both y-down pixels. */
  struct Vec2 {
    double x = 0.0;
    double y = 0.0;
    friend constexpr bool operator==(const Vec2&, const Vec2&) = default;
  };

  constexpr Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
  constexpr Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
  constexpr Vec2 operator*(Vec2 v, double s) { return {v.x * s, v.y * s}; }
  constexpr Vec2 operator/(Vec2 v, double s) { return {v.x / s, v.y / s}; }

  /** 2D vector in whole grid units. */
  struct Vec2i {
    int x = 0;
    int y = 0;
    friend constexpr bool operator==(const Vec2i&, const Vec2i&) = default;
  };

  constexpr Vec2i operator+(Vec2i a, Vec2i b) { return {a.x + b.x, a.y + b.y}; }

  /** Axis-aligned rectangle: top-left (x,y) + size (w,h), y-down. */
  struct Rect {
    double x = 0.0;
    double y = 0.0;
    double w = 0.0;
    double h = 0.0;

    constexpr Vec2 topLeft() const { return {x, y}; }
    constexpr Vec2 center() const { return {x + w * 0.5, y + h * 0.5}; }
    constexpr bool contains(Vec2 point) const {
      return point.x >= x && point.y >= y && point.x < x + w && point.y < y + h;
    }

    friend constexpr bool operator==(const Rect&, const Rect&) = default;
  };

  /** Overlapping region of two rects, or nullopt when they do not overlap in area. */
  constexpr std::optional<Rect> intersect(const Rect& a, const Rect& b) {
    const double left = std::max(a.x, b.x);
    const double top = std::max(a.y, b.y);
    const double right = std::min(a.x + a.w, b.x + b.w);
    const double bottom = std::min(a.y + a.h, b.y + b.h);
    if (right <= left || bottom <= top) return std::nullopt;
    return Rect{.x = left, .y = top, .w = right - left, .h = bottom - top};
  }

  template <typename T>
  struct Limits {
    T lower;
    T upper;
  };

}  // namespace fluir::editor
