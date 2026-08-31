#include "editor/core/render_graph.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <variant>
#include <vector>

namespace fluir::editor {

  namespace {

    // Text inset from a rect's top-left corner (world px).
    constexpr double kTextPad = 4.0;
    // Side length of a port dot (world px).
    constexpr double kPortDot = 6.0;

    // Client constants ported from editor/client: FUNC_HEADER_HEIGHT == 5 and
    // PARAM_BLOCK_HEIGHT == 5 logical units; RETURN_NODE_WIDTH == 5 logical.
    // World px = logical * UNIT_PX (== ZOOM_SCALAR == 5).
    constexpr double kHeaderH = 5.0 * UNIT_PX;   // 25
    constexpr double kRailStep = 5.0 * UNIT_PX;  // 25 — row height and y-step
    constexpr double kParamW = 15.0 * UNIT_PX;   // 75
    constexpr double kReturnInsetLogical = 5.0;  // rail sits at logical x = width - 5

    Rect dotRect(Vec2 anchor) { return {anchor.x - kPortDot * 0.5, anchor.y - kPortDot * 0.5, kPortDot, kPortDot}; }

    Vec2 toScreen(const Viewport& vp, Vec2 world) { return vp.worldToScreen(world); }

    Rect toScreen(const Viewport& vp, Rect world) {
      const Vec2 tl = vp.worldToScreen(world.topLeft());
      return {tl.x, tl.y, world.w * vp.scale, world.h * vp.scale};
    }

    // Deterministic order: (location.z, id).
    std::vector<const pt::FunctionDecl*> sortedFunctions(const pt::ParseTree& tree) {
      std::vector<const pt::FunctionDecl*> funcs;
      funcs.reserve(tree.declarations.size());
      for (const auto& entry : tree.declarations) {
        if (const auto* fn = std::get_if<pt::FunctionDecl>(&entry.second)) {
          funcs.push_back(fn);
        }
      }
      std::sort(funcs.begin(), funcs.end(), [](const pt::FunctionDecl* a, const pt::FunctionDecl* b) {
        if (a->location.z != b->location.z) {
          return a->location.z < b->location.z;
        }
        return a->id < b->id;
      });
      return funcs;
    }

    void emitParamRail(const pt::FunctionDecl::InputBlock& input, Vec2 origin, const Viewport& vp, Renderer& out) {
      std::vector<const pt::FunctionDecl::Parameter*> params;
      params.reserve(input.parameters.size());
      for (const auto& param : input.parameters) {
        params.push_back(&param);
      }
      std::sort(
        params.begin(), params.end(), [](const pt::FunctionDecl::Parameter* a, const pt::FunctionDecl::Parameter* b) {
          return a->index < b->index;
        });

      for (std::size_t i = 0; i < params.size(); ++i) {
        const pt::FunctionDecl::Parameter& param = *params[i];
        const Rect rect{origin.x, origin.y + (static_cast<double>(i) + 1.0) * kRailStep, kParamW, kRailStep};
        out.drawRect(toScreen(vp, rect));
        out.drawText(toScreen(vp, Vec2{rect.x + kTextPad, rect.y + kTextPad}), param.typeName + " " + param.name);
        const Vec2 anchor{rect.x + rect.w, rect.y + rect.h * 0.5};
        out.fillRect(toScreen(vp, dotRect(anchor)));
      }
    }

    void emitReturnRail(
      const pt::FunctionDecl::Return& ret, Vec2 origin, int width, const Viewport& vp, Renderer& out) {
      const Rect rect{origin.x + (static_cast<double>(width) - kReturnInsetLogical) * UNIT_PX,
                      origin.y + kRailStep,
                      kRailStep,
                      kRailStep};
      out.drawRect(toScreen(vp, rect));
      out.drawText(toScreen(vp, Vec2{rect.x + kTextPad, rect.y + kTextPad}), ret.typeName);
      const Vec2 anchor{rect.x, rect.y + rect.h * 0.5};
      out.fillRect(toScreen(vp, dotRect(anchor)));
    }

  }  // namespace

  void renderGraph(const pt::ParseTree& tree, const Viewport& view, Renderer& renderer) {
    for (const pt::FunctionDecl* fn : sortedFunctions(tree)) {
      const FlowGraphLocation& loc = fn->location;
      const Vec2 origin{loc.x * UNIT_PX, loc.y * UNIT_PX};

      const Rect frame{origin.x, origin.y, loc.width * UNIT_PX, loc.height * UNIT_PX};
      renderer.drawRect(toScreen(view, frame));

      const Rect header{origin.x, origin.y, loc.width * UNIT_PX, kHeaderH};
      renderer.fillRect(toScreen(view, header));
      renderer.drawText(toScreen(view, Vec2{origin.x + kTextPad, origin.y + kTextPad}), fn->name);

      if (fn->input) {
        emitParamRail(*fn->input, origin, view, renderer);
      }
      if (fn->output && fn->output->ret) {
        emitReturnRail(*fn->output->ret, origin, loc.width, view, renderer);
      }
    }
  }

}  // namespace fluir::editor
