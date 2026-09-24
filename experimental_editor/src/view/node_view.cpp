#include "editor/view/node_view.hpp"

#include <variant>

#include "editor/view/draw/binary.hpp"
#include "editor/view/draw/call.hpp"
#include "editor/view/draw/comment.hpp"
#include "editor/view/draw/conditional.hpp"
#include "editor/view/draw/constant.hpp"
#include "editor/view/draw/unary.hpp"

namespace fluir::editor {

  TerminalSet terminals(const pt::Node& node, Rect world, const EditorContext::Layout& layout) {
    return std::visit([&](const auto& n) { return draw::anchors(n, world, layout); }, node);
  }

  std::vector<FieldLabel> nodeLabels(const pt::Node& node, Rect world, const EditorContext::Layout& layout) {
    return std::visit([&](const auto& n) { return draw::labels(n, world, layout); }, node);
  }

  Limits<Vec2i> nodeSizeLimits(const pt::Node& node) {
    return std::visit([](const auto& n) { return draw::sizeLimits(n); }, node);
  }

  std::optional<Part> nodeResizePart(const pt::Node& node) {
    return std::visit([](const auto& n) { return draw::resizePart(n); }, node);
  }

  Color nodeColor(const pt::Node& node, const EditorContext::Theme& theme) {
    return std::visit([&](const auto& n) { return draw::color(n, theme); }, node);
  }

  void drawNode(const pt::Node& node, Rect world, const Subview& view, const EditorContext& ctx) {
    std::visit([&](const auto& n) { draw::draw(n, world, view, ctx); }, node);
  }

}  // namespace fluir::editor
