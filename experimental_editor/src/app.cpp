#include "editor/app.hpp"

#include <cmath>

#include <SDL3/SDL.h>

#include "compiler/utility/context.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/loader.hpp"
#include "editor/core/render_graph.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"
#include "editor/pages/module.hpp"

namespace fluir::editor {

  int run(EditorContext& ctx, Renderer& renderer) {
    ModulePage page{ctx, renderer};
    // TODO: Create input system and drive event loop
    return page.run();
  }

}  // namespace fluir::editor
