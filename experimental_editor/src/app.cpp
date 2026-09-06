#include "editor/app.hpp"

#include <chrono>
#include <memory>

#include "editor/input.hpp"
#include "editor/pages/splash.hpp"

namespace fluir::editor {
  using namespace std::chrono_literals;

  int run(EditorContext& ctx, Renderer& renderer) {
    InputManager input;
    std::unique_ptr<Page> page = std::make_unique<SplashPage>(ctx, renderer);

    if (const int error = page->start(); error) {
      return error;
    }

    while (ctx.running) {
      const auto events = input.read(10ms);
      if (const int error = page->update(events); error) {
        return error;
      }
      if (auto next = page->next()) {
        page = std::move(next);
        if (const int error = page->start(); error) {
          return error;
        }
        continue;  // let the new page's own draw() happen next iteration
      }
      if (const int error = page->draw(); error) {
        return error;
      }
    }

    return 0;
  }
}  // namespace fluir::editor
