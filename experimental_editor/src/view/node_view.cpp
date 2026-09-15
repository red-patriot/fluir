#include "editor/view/node_view.hpp"

#include <variant>

#include "editor/view/draw/binary.hpp"
#include "editor/view/draw/call.hpp"
#include "editor/view/draw/comment.hpp"
#include "editor/view/draw/constant.hpp"
#include "editor/view/draw/unary.hpp"

namespace fluir::editor {

  PortSet ports(const pt::Node& node, Rect world, const EditorContext::Layout& layout) {
    return std::visit([&](const auto& n) { return draw::anchors(n, world, layout); }, node);
  }

  Color nodeColor(const pt::Node& node, const EditorContext::Theme& theme) {
    return std::visit([&](const auto& n) { return draw::color(n, theme); }, node);
  }

  void drawNode(const pt::Node& node, Rect world, const Subview& view, const EditorContext& ctx) {
    std::visit([&](const auto& n) { draw::draw(n, world, view, ctx); }, node);
  }

}  // namespace fluir::editor
