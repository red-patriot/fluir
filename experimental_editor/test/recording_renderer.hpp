#pragma once

#include <string>
#include <vector>

#include "../include/editor/core/renderer.hpp"

namespace testutil {

  struct DrawCall {
    enum class Op { Rect, Fill, Line, Text, PushClip, PopClip };
    Op op;
    fluir::editor::Rect rect;  // Rect / Fill / PushClip
    fluir::editor::Vec2 a;     // Line a / Text pos
    fluir::editor::Vec2 b;     // Line b
    std::string text;          // Text
    friend bool operator==(const DrawCall&, const DrawCall&) = default;
  };

  class RecordingRenderer : public fluir::editor::Renderer {
   public:
    std::vector<DrawCall> calls;

    void drawRect(fluir::editor::Rect r) override { calls.push_back({DrawCall::Op::Rect, r, {}, {}, {}}); }
    void fillRect(fluir::editor::Rect r) override { calls.push_back({DrawCall::Op::Fill, r, {}, {}, {}}); }
    void drawLine(fluir::editor::Vec2 p, fluir::editor::Vec2 q) override {
      calls.push_back({DrawCall::Op::Line, {}, p, q, {}});
    }
    void drawText(fluir::editor::Vec2 pos, std::string_view t) override {
      calls.push_back({DrawCall::Op::Text, {}, pos, {}, std::string{t}});
    }
    void pushClip(fluir::editor::Rect r) override { calls.push_back({DrawCall::Op::PushClip, r, {}, {}, {}}); }
    void popClip() override { calls.push_back({DrawCall::Op::PopClip, {}, {}, {}, {}}); }
  };

}  // namespace testutil
