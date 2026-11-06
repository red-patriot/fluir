#include "editor/view/draw/draw_utils.hpp"

#include <algorithm>
#include <QFontMetricsF>
#include <QString>

#include "editor/scene/item_style.hpp"

namespace fluir::editor {
  namespace {

    // Grip sizes, in grid units.
    constexpr double DRAG_SIZE = 3;
    constexpr double DRAG_INSET = 1;
    constexpr double RESIZE_CORNER_SIZE = 3;
    constexpr double RESIZE_EDGE_THICKNESS = 0.8;

    // A lone terminal sits at the centre; more spread top-to-bottom down the edge.
    double terminalFraction(int count, int index) {
      return count <= 1 ? 0.5 : static_cast<double>(index) / (static_cast<double>(count) - 1.0);
    }

  }  // namespace

  Rect moveGrip(const Rect& frame, double unit) {
    return {frame.x + frame.w - (DRAG_SIZE + DRAG_INSET) * unit,
            frame.y + DRAG_INSET * unit,
            DRAG_SIZE * unit,
            DRAG_SIZE * unit};
  }

  Rect resizeEdge(const Rect& frame, double unit) {
    return {frame.x + frame.w - RESIZE_EDGE_THICKNESS * unit, frame.y, RESIZE_EDGE_THICKNESS * unit, frame.h};
  }

  Rect resizeCorner(const Rect& frame, double unit) {
    const double size = RESIZE_CORNER_SIZE * unit;
    return {frame.x + frame.w - size, frame.y + frame.h - size, size, size};
  }

  Rect fitInto(Rect target, Vec2 intrinsic) {
    if (intrinsic.x <= 0.0 || intrinsic.y <= 0.0 || target.w <= 0.0 || target.h <= 0.0) {
      return {target.center().x, target.center().y, 0.0, 0.0};
    }
    const double scale = std::min(target.w / intrinsic.x, target.h / intrinsic.y);
    const Vec2 size = intrinsic * scale;
    return {target.x + (target.w - size.x) / 2, target.y + (target.h - size.y) / 2, size.x, size.y};
  }

  SplitLabel splitLabel(Rect box, std::string_view tag, const EditorContext::Layout& layout) {
    const QString text = QString::fromUtf8(tag.data(), static_cast<qsizetype>(tag.size()));
    const double tagW = 2 * layout.textPad + QFontMetricsF(style::tagFont()).horizontalAdvance(text);
    return {Rect{box.x, box.y, tagW, box.h}, Rect{box.x + tagW, box.y, std::max(0.0, box.w - tagW), box.h}};
  }

  namespace draw {

    Rect selectionOutline(const Rect& frame, const EditorContext::Layout& layout) {
      const double out = layout.selectionPad + 2;
      return {frame.x - out, frame.y - out, frame.w + 2 * out, frame.h + 2 * out};
    }

    std::vector<Vec2> edgeAnchors(double edgeX, const Rect& rect, int count) {
      std::vector<Vec2> out;
      for (int i = 0; i < count; ++i) {
        out.push_back(Vec2{edgeX, rect.y + terminalFraction(count, i) * rect.h});
      }
      return out;
    }

  }  // namespace draw

}  // namespace fluir::editor
