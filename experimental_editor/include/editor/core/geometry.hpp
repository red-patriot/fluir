#pragma once

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

  /** Axis-aligned rectangle: top-left (x,y) + size (w,h), y-down. */
  struct Rect {
    double x = 0.0;
    double y = 0.0;
    double w = 0.0;
    double h = 0.0;

    constexpr Vec2 topLeft() const { return {x, y}; }
    constexpr Vec2 center() const { return {x + w * 0.5, y + h * 0.5}; }

    friend constexpr bool operator==(const Rect&, const Rect&) = default;
  };

}  // namespace fluir::editor
