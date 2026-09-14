#include "editor/view/graph_draw.hpp"

#include <string_view>
#include <variant>

#include "editor/core/graph_geometry.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/view/node_view.hpp"

namespace fluir::editor {
  namespace {

    constexpr std::string_view kFnTag = "fn";

    // Renderer::drawRect has no thickness, so two concentric rects stand in for a 2px outline.
    void drawOutline(const Subview& view, const EditorContext& ctx, const Rect& r) {
      for (const double pad : {ctx.layout.selectionPad, ctx.layout.selectionPad + 1.0}) {
        view.renderer().drawRect(view.toScreen(Rect{r.x - pad, r.y - pad, r.w + 2 * pad, r.h + 2 * pad}),
                                 ctx.theme.border);
      }
    }

    // A parameter's port is on its right edge, the return's on its left.
    void drawRail(
      const Subview& view, const pt::FunctionDecl& fn, fluir::ID id, const Rect& r, const EditorContext& ctx) {
      const std::string* typeName = railTypeAt(fn, id);
      std::string_view name;
      Vec2 anchor{r.x + r.w, r.y + r.h * 0.5};
      if (fn.output && fn.output->ret && fn.output->ret->id == id) {
        anchor = Vec2{r.x, r.y + r.h * 0.5};
      } else if (fn.input) {
        for (const auto& param : fn.input->parameters) {
          if (param.id == id) {
            name = param.name;
          }
        }
      }
      Renderer& renderer = view.renderer();
      renderer.fillRect(view.toScreen(r), ctx.theme.funcDeclHeader);
      renderer.drawRect(view.toScreen(r), ctx.theme.border);
      drawSplitLabel(view, r, typeName == nullptr ? std::string_view{} : *typeName, name, ctx);
      renderer.fillRect(view.toScreen(dotRect(anchor, ctx.layout.portDot)), ctx.theme.border);
    }

    void drawBox(
      const Subview& view, const pt::ParseTree& tree, const Box& box, bool selected, const EditorContext& ctx) {
      Renderer& r = view.renderer();
      switch (box.part) {
        case Part::Body:
          if (functionAt(tree, box.path) != nullptr) {
            r.fillRect(view.toScreen(box.world), ctx.theme.background);
          } else if (const auto* comment = std::get_if<pt::Comment>(declarationAt(tree, box.path))) {
            drawComment(*comment, box.world, view, ctx);
            if (selected) {
              drawOutline(view, ctx, box.world);
            }
          } else if (const pt::Node* node = nodeAt(tree, box.path)) {
            drawNode(*node, box.world, view, ctx);
            if (selected) {
              drawOutline(view, ctx, box.world);
            }
          }
          return;
        case Part::Frame:
          if (const pt::FunctionDecl* fn = functionAt(tree, box.path)) {
            const Rect& f = box.world;
            r.fillRect(view.toScreen(Rect{f.x, f.y, f.w, ctx.layout.headerH()}), ctx.theme.funcDeclHeader);
            r.drawRect(view.toScreen(f), ctx.theme.border);
            drawSplitLabel(view, Rect{f.x, f.y, f.w, ctx.layout.headerH()}, kFnTag, fn->name, ctx);
            if (selected) {
              drawOutline(view, ctx, f);
            }
          }
          return;
        case Part::Rail:
          if (const pt::FunctionDecl* fn = functionAt(tree, parentOf(box.path))) {
            drawRail(view, *fn, box.path.back(), box.world, ctx);
          }
          return;
        case Part::Wire:
          r.drawLine(view.toScreen(box.world.topLeft()),
                     view.toScreen(Vec2{box.world.x + box.world.w, box.world.y + box.world.h}),
                     ctx.theme.conduit);
          return;
        case Part::MoveGrip:
        case Part::ResizeX:
        case Part::ResizeXY:
          r.fillRect(view.toScreen(box.world), ctx.theme.border);
          return;
      }
    }

  }  // namespace

  void drawGraph(const Subview& view,
                 const pt::ParseTree& tree,
                 std::span<const Box> boxes,
                 const std::optional<FullID>& selection,
                 const EditorContext& ctx) {
    for (const Box& box : boxes) {
      if (box.clip) {
        view.renderer().pushClip(view.toScreen(*box.clip));
      }
      drawBox(view, tree, box, selection == box.path, ctx);
      if (box.clip) {
        view.renderer().popClip();
      }
    }
  }

}  // namespace fluir::editor
