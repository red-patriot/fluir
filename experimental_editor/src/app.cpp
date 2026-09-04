#include "editor/app.hpp"

#include <chrono>

#include "editor/input.hpp"
#include "editor/pages/module.hpp"

namespace fluir::editor {
  using namespace std::chrono_literals;

  int run(EditorContext& ctx, Renderer& renderer) {
    InputManager input;
    ModulePage page{ctx, renderer};

    if (const int error = page.start(); error) {
      return error;
    }

    while (ctx.running) {
      const auto events = input.read(10ms);                // Read
      if (const int error = page.update(events); error) {  // Update
        return error;
      }
      if (const int error = page.write(); error) {  // Write
        return error;
      }
    }

    return 0;
  }

}  // namespace fluir::editor
