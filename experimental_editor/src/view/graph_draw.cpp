#include "editor/view/graph_draw.hpp"

#include <variant>

#include "editor/core/renderer.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/view/draw/comment.hpp"
#include "editor/view/draw/conditional.hpp"
#include "editor/view/draw/function.hpp"
#include "editor/view/graph_geometry.hpp"
#include "editor/view/node_view.hpp"
#include "fluir/util/overloaded.hpp"

namespace fluir::editor {
  namespace {

    // Renderer::drawRect has no thickness, so two concentric rects stand in for a 2px outline.
    void drawOutline(const Subview& view, const EditorContext& ctx, const Rect& r) {
      for (const double pad : {ctx.layout.selectionPad, ctx.layout.selectionPad + 1.0}) {
        view.renderer().drawRect(view.toScreen(Rect{r.x - pad, r.y - pad, r.w + 2 * pad, r.h + 2 * pad}),
                                 ctx.theme.border);
      }
    }

    // What the path names decides the body: a declaration, a node, or a container's branch.
    void drawBody(const Subview& view, const pt::ParseTree& tree, const Box& box, const EditorContext& ctx) {
      if (const pt::Declaration* decl = declarationAt(tree, box.path)) {
        std::visit(util::Overloaded{[&](const pt::FunctionDecl& fn) { draw::drawBody(fn, box.world, view, ctx); },
                                    [&](const pt::Comment& comment) { draw::draw(comment, box.world, view, ctx); }},
                   *decl);
      } else if (const pt::Node* node = nodeAt(tree, box.path)) {
        drawNode(*node, box.world, view, ctx);
      }
    }

    // A container's selection outline belongs to its frame, which paints over its children.
    bool framed(const pt::ParseTree& tree, const FullID& path) {
      return functionAt(tree, path) != nullptr || std::get_if<pt::Conditional>(nodeAt(tree, path)) != nullptr;
    }

    void drawBox(
      const Subview& view, const pt::ParseTree& tree, const Box& box, bool selected, const EditorContext& ctx) {
      Renderer& r = view.renderer();
      switch (box.part) {
        case Part::Body:
          drawBody(view, tree, box, ctx);
          if (selected && !framed(tree, box.path)) {
            drawOutline(view, ctx, box.world);
          }
          return;
        case Part::Branch:
        case Part::Header:
        case Part::Label:
          return;  // hit regions only: their owner paints them
        case Part::Frame:
          if (const pt::FunctionDecl* fn = functionAt(tree, box.path)) {
            draw::drawFrame(*fn, box.world, view, ctx);
          } else if (const auto* conditional = std::get_if<pt::Conditional>(nodeAt(tree, box.path))) {
            draw::drawFrame(*conditional, box.world, view, ctx);
          } else {
            return;
          }
          if (selected) {
            drawOutline(view, ctx, box.world);
          }
          return;
        case Part::Rail:
          if (const pt::FunctionDecl* fn = functionAt(tree, parentOf(box.path))) {
            draw::drawRail(*fn, box.path.back(), box.world, view, ctx);
          }
          return;
        case Part::Wire:
          r.drawLine(view.toScreen(box.world.topLeft()),
                     view.toScreen(Vec2{box.world.x + box.world.w, box.world.y + box.world.h}),
                     ctx.theme.conduit);
          return;
        case Part::MoveGrip:
          draw::drawMoveGrip(box.world, view, ctx);
          return;
        case Part::ResizeX:
        case Part::ResizeY:
          draw::drawResizeEdge(box.world, view, ctx);
          return;
        case Part::ResizeXY:
          draw::drawXyResizeHandle(box.world, view, ctx);
          return;
      }
    }

  }  // namespace

  void drawGraph(const Subview& view,
                 const pt::ParseTree& tree,
                 std::span<const Box> boxes,
                 const std::optional<FullID>& selection,
                 const EditorContext& ctx) {
    // A branch has no outline of its own: selecting one outlines the container it belongs to.
    const std::optional<FullID> outlined =
      selection && isBranchPath(*selection) ? std::optional<FullID>{parentOf(*selection)} : selection;
    for (const Box& box : boxes) {
      if (box.clip) {
        view.renderer().pushClip(view.toScreen(*box.clip));
      }
      drawBox(view, tree, box, outlined == box.path, ctx);
      if (box.clip) {
        view.renderer().popClip();
      }
    }
  }

}  // namespace fluir::editor
