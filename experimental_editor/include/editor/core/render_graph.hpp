#pragma once

#include <unordered_map>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  // Draws a Parse Tree's elements to a Renderer
  class GraphRenderer {
   public:
    GraphRenderer(const Viewport& viewport, Renderer& renderer);

    void operator()(const pt::ParseTree& tree);

    void operator()(const pt::FunctionDecl& decl);

    void operator()(const pt::Binary& binary);
    void operator()(const pt::Unary& unary);
    void operator()(const pt::Constant& constant);
    void operator()(const pt::Call& call);

    void operator()(const pt::Conduit& conduit);

   private:
    struct PortSet {
      std::vector<Vec2> inputs;
      std::vector<Vec2> outputs;
    };
    using PortMap = std::unordered_map<fluir::ID, PortSet>;

    void drawParamRail(const pt::FunctionDecl::InputBlock& input);
    void drawReturnRail(const pt::FunctionDecl::Return& ret);

    Viewport viewport_;
    Renderer& renderer_;

    // Current function's world origin, logical width, and ports
    PortMap ports_;
    Vec2 origin_{};
    int width_ = 0;
  };

  /** Walk `tree` and draw to renderer */
  void renderGraph(const pt::ParseTree& tree, const Viewport& view, Renderer& renderer);

  /** World-space axis-aligned bounding box of every function frame in `tree`
   *  ({0,0,0,0} when there are no functions). Used to fit the view. */
  Rect graphBounds(const pt::ParseTree& tree);

}  // namespace fluir::editor
