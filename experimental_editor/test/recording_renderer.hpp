#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/renderer.hpp"

namespace testutil {

  struct DrawCall {
    enum class Op { Rect, Fill, Line, Text, TextWrapped, Icon, PushClip, PopClip };
    Op op;
    fluir::editor::Rect rect;            // Rect / Fill / PushClip / TextWrapped
    fluir::editor::Vec2 a;               // Line a / Text pos
    fluir::editor::Vec2 b;               // Line b
    std::string text;                    // Text / TextWrapped
    double scale = 1.0;                  // Text / TextWrapped
    const unsigned char* svg = nullptr;  // Icon: the embedded bytes' address, which identifies the icon
    friend bool operator==(const DrawCall&, const DrawCall&) = default;
  };

  class RecordingRenderer : public fluir::editor::Renderer {
   public:
    std::vector<DrawCall> calls;
    fluir::editor::Vec2 outputSize_{800, 600};  // test-settable

    void beginFrame() override { }
    void endFrame() override { }

    fluir::editor::Vec2 outputSize() override { return outputSize_; }

    void drawRect(fluir::editor::Rect r, const fluir::editor::Color&) override {
      calls.push_back({DrawCall::Op::Rect, r, {}, {}, {}});
    }
    void fillRect(fluir::editor::Rect r, const fluir::editor::Color&) override {
      calls.push_back({DrawCall::Op::Fill, r, {}, {}, {}});
    }
    void drawLine(fluir::editor::Vec2 p, fluir::editor::Vec2 q, const fluir::editor::Color&) override {
      calls.push_back({DrawCall::Op::Line, {}, p, q, {}});
    }
    void drawText(fluir::editor::Vec2 pos,
                  std::string_view t,
                  const fluir::editor::Color&,
                  double scale = 1.0) override {
      calls.push_back({DrawCall::Op::Text, {}, pos, {}, std::string{t}, scale});
    }
    void drawTextWrapped(fluir::editor::Rect r,
                         std::string_view t,
                         double scale,
                         const fluir::editor::Color&) override {
      calls.push_back({DrawCall::Op::TextWrapped, r, {}, {}, std::string{t}, scale});
    }
    // Monospace fallback for wrapped layout: 8*scale cells, wrapping per character.
    std::size_t wrappedIndexAt(fluir::editor::Rect r,
                               std::string_view t,
                               double scale,
                               fluir::editor::Vec2 p) override {
      const double cell = 8.0 * scale;
      const double cols = wrapColumns(r, cell);
      const double row = std::max(0.0, std::floor((p.y - r.y) / cell));
      const double col = std::clamp(std::round((p.x - r.x) / cell), 0.0, cols);
      return std::min(static_cast<std::size_t>(row * cols + col), t.size());
    }
    fluir::editor::Rect wrappedCaretRect(fluir::editor::Rect r,
                                         std::string_view t,
                                         double scale,
                                         std::size_t index) override {
      const double cell = 8.0 * scale;
      const auto cols = static_cast<std::size_t>(wrapColumns(r, cell));
      const std::size_t i = std::min(index, t.size());
      return {r.x + static_cast<double>(i % cols) * cell, r.y + static_cast<double>(i / cols) * cell, 1.0, cell};
    }
    static double wrapColumns(fluir::editor::Rect r, double cell) { return std::max(1.0, std::floor(r.w / cell)); }
    fluir::editor::Vec2 measureText(std::string_view t) override { return {static_cast<double>(t.size()) * 8.0, 8.0}; }
    void drawIcon(fluir::editor::Rect r, std::span<const unsigned char> svg, const fluir::editor::Color&) override {
      calls.push_back({DrawCall::Op::Icon, r, {}, {}, {}, 1.0, svg.data()});
    }
    // Square by default, so `fitInto` centres predictably in tests.
    fluir::editor::Vec2 imageSize(std::span<const unsigned char>) override { return {16.0, 16.0}; }
    void pushClip(fluir::editor::Rect r) override { calls.push_back({DrawCall::Op::PushClip, r, {}, {}, {}}); }
    void popClip() override { calls.push_back({DrawCall::Op::PopClip, {}, {}, {}, {}}); }
  };

  inline void expectRectNear(const fluir::editor::Rect& got, const fluir::editor::Rect& want, double tol = 1e-6) {
    EXPECT_NEAR(got.x, want.x, tol);
    EXPECT_NEAR(got.y, want.y, tol);
    EXPECT_NEAR(got.w, want.w, tol);
    EXPECT_NEAR(got.h, want.h, tol);
  }

  inline void expectVecNear(const fluir::editor::Vec2& got, const fluir::editor::Vec2& want, double tol = 1e-6) {
    EXPECT_NEAR(got.x, want.x, tol);
    EXPECT_NEAR(got.y, want.y, tol);
  }

  namespace detail {

    inline bool rectNear(const fluir::editor::Rect& a, const fluir::editor::Rect& b, double tol) {
      return std::abs(a.x - b.x) <= tol && std::abs(a.y - b.y) <= tol && std::abs(a.w - b.w) <= tol &&
             std::abs(a.h - b.h) <= tol;
    }

    inline bool vecNear(const fluir::editor::Vec2& a, const fluir::editor::Vec2& b, double tol) {
      return std::abs(a.x - b.x) <= tol && std::abs(a.y - b.y) <= tol;
    }

  }  // namespace detail

  // All `Op`-kind draw calls, in emission order (callers must not rely on that order).
  inline std::vector<DrawCall> opsOf(const std::vector<DrawCall>& calls, DrawCall::Op op) {
    std::vector<DrawCall> out;
    for (const auto& c : calls) {
      if (c.op == op) {
        out.push_back(c);
      }
    }
    return out;
  }

  // Genuine cardinality: how many draw calls of this kind were made.
  inline std::size_t countOf(const std::vector<DrawCall>& calls, DrawCall::Op op) { return opsOf(calls, op).size(); }

  inline std::vector<fluir::editor::Rect> rectsOf(const std::vector<DrawCall>& calls) {
    std::vector<fluir::editor::Rect> out;
    for (const auto& c : opsOf(calls, DrawCall::Op::Rect)) {
      out.push_back(c.rect);
    }
    return out;
  }

  inline std::vector<fluir::editor::Rect> fillsOf(const std::vector<DrawCall>& calls) {
    std::vector<fluir::editor::Rect> out;
    for (const auto& c : opsOf(calls, DrawCall::Op::Fill)) {
      out.push_back(c.rect);
    }
    return out;
  }

  // Every string drawn as text, for "text set == {...}" contracts.
  inline std::vector<std::string> textStrings(const std::vector<DrawCall>& calls) {
    std::vector<std::string> out;
    for (const auto& c : opsOf(calls, DrawCall::Op::Text)) {
      out.push_back(c.text);
    }
    return out;
  }

  inline bool hasRect(const std::vector<DrawCall>& calls, fluir::editor::Rect want, double tol = 1e-6) {
    for (const auto& r : rectsOf(calls)) {
      if (detail::rectNear(r, want, tol)) {
        return true;
      }
    }
    return false;
  }

  // Icons are matched by their bytes' address: the embedded array is the icon's identity.
  inline bool hasIcon(const std::vector<DrawCall>& calls,
                      std::span<const unsigned char> svg,
                      fluir::editor::Rect want,
                      double tol = 1e-6) {
    for (const auto& c : opsOf(calls, DrawCall::Op::Icon)) {
      if (c.svg == svg.data() && detail::rectNear(c.rect, want, tol)) {
        return true;
      }
    }
    return false;
  }

  inline bool hasFill(const std::vector<DrawCall>& calls, fluir::editor::Rect want, double tol = 1e-6) {
    for (const auto& r : fillsOf(calls)) {
      if (detail::rectNear(r, want, tol)) {
        return true;
      }
    }
    return false;
  }

  inline bool hasTextAt(const std::vector<DrawCall>& calls,
                        std::string_view s,
                        fluir::editor::Vec2 at,
                        double tol = 1e-6) {
    for (const auto& c : opsOf(calls, DrawCall::Op::Text)) {
      if (c.text == s && detail::vecNear(c.a, at, tol)) {
        return true;
      }
    }
    return false;
  }

  inline bool hasScaledTextAt(
    const std::vector<DrawCall>& calls, std::string_view s, fluir::editor::Vec2 at, double scale, double tol = 1e-6) {
    for (const auto& c : opsOf(calls, DrawCall::Op::Text)) {
      if (c.text == s && detail::vecNear(c.a, at, tol) && std::abs(c.scale - scale) <= tol) {
        return true;
      }
    }
    return false;
  }

  inline bool hasWrappedText(
    const std::vector<DrawCall>& calls, std::string_view s, fluir::editor::Rect rect, double scale, double tol = 1e-6) {
    for (const auto& c : opsOf(calls, DrawCall::Op::TextWrapped)) {
      if (c.text == s && detail::rectNear(c.rect, rect, tol) && std::abs(c.scale - scale) <= tol) {
        return true;
      }
    }
    return false;
  }

  // Order-independent: matches either p->q or q->p.
  inline bool hasLine(const std::vector<DrawCall>& calls,
                      fluir::editor::Vec2 p,
                      fluir::editor::Vec2 q,
                      double tol = 1e-6) {
    for (const auto& c : opsOf(calls, DrawCall::Op::Line)) {
      if ((detail::vecNear(c.a, p, tol) && detail::vecNear(c.b, q, tol)) ||
          (detail::vecNear(c.a, q, tol) && detail::vecNear(c.b, p, tol))) {
        return true;
      }
    }
    return false;
  }

  // All PushClip rects matching `want`; callers assert `.size() == 1` for a
  // single-clip contract.
  inline std::vector<fluir::editor::Rect> clipsCovering(const std::vector<DrawCall>& calls,
                                                        fluir::editor::Rect want,
                                                        double tol = 1e-6) {
    std::vector<fluir::editor::Rect> out;
    for (const auto& c : opsOf(calls, DrawCall::Op::PushClip)) {
      if (detail::rectNear(c.rect, want, tol)) {
        out.push_back(c.rect);
      }
    }
    return out;
  }

  // Fills of an exact w x h (port dots are 6x6): lets a test assert the set of
  // dot positions instead of a bare count.
  inline std::vector<fluir::editor::Rect> fillsOfSize(const std::vector<DrawCall>& calls,
                                                      double w,
                                                      double h,
                                                      double tol = 1e-6) {
    std::vector<fluir::editor::Rect> out;
    for (const auto& r : fillsOf(calls)) {
      if (std::abs(r.w - w) <= tol && std::abs(r.h - h) <= tol) {
        out.push_back(r);
      }
    }
    return out;
  }

  // Membership check against a rect list (e.g. the result of fillsOfSize), so
  // callers can assert "this exact dot is among the recorded ones".
  inline bool containsRect(const std::vector<fluir::editor::Rect>& rects, fluir::editor::Rect want, double tol = 1e-6) {
    for (const auto& r : rects) {
      if (detail::rectNear(r, want, tol)) {
        return true;
      }
    }
    return false;
  }

}  // namespace testutil
