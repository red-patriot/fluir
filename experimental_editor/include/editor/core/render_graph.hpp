#pragma once

#include <unordered_map>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/actors/actor.hpp"
#include "editor/actors/scene.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  // Draws a Parse Tree's elements to a Renderer
  class GraphRenderer {
   public:
    GraphRenderer(const EditorContext& ctx, const Viewport& viewport, Renderer& renderer, const GraphScene& scene);

    void operator()(const pt::ParseTree& tree);

    void operator()(const pt::FunctionDecl& decl);

    void operator()(const pt::Conduit& conduit);

   private:
    using PortMap = std::unordered_map<fluir::ID, PortSet>;

    void drawParamRail(Vec2 origin, const pt::FunctionDecl::InputBlock& input);
    void drawReturnRail(Vec2 origin, const pt::FunctionDecl::Return& ret, int width);

    const EditorContext& ctx_;
    Viewport viewport_;
    Renderer& renderer_;
    const GraphScene& scene_;

    // Current function's ports, and the body Subview.
    PortMap ports_;
    const Subview* body_ = nullptr;
  };

  /** Walk `tree` and draw to renderer */
  void renderGraph(const EditorContext& ctx, const pt::ParseTree& tree, const Viewport& view, Renderer& renderer);

  /** World-space axis-aligned bounding box of every function frame in `tree`
   *  ({0,0,0,0} when there are no functions). Used to fit the view. */
  Rect graphBounds(const EditorContext& ctx, const pt::ParseTree& tree);

}  // namespace fluir::editor
