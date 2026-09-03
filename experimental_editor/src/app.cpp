#include "editor/app.hpp"

#include <cmath>

#include <SDL3/SDL.h>

#include "editor/core/render_graph.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"

namespace fluir::editor {
  static constexpr double ZOOM_UPPER_BOUND = 2.5;
  static constexpr double ZOOM_LOWER_BOUND = 0.25;

  int run(const pt::ParseTree& tree, Renderer& renderer, Vec2 viewportSize) {
    Viewport vp;
    vp.fitRect(graphBounds(tree), viewportSize);

    const auto redraw = [&] {
      renderer.beginFrame();
      renderGraph(tree, vp, renderer);
      renderer.endFrame();
    };
    redraw();

    bool panning = false;
    bool spaceHeld = false;
    Vec2 lastPan;

    SDL_Event e;
    while (SDL_WaitEvent(&e)) {
      const auto ie = translate(e);
      if (!ie) {
        continue;
      }

      bool stop = false;
      switch (ie->type) {
        case InputEvent::Type::Quit:
          stop = true;
          break;

        case InputEvent::Type::KeyDown:
          if (ie->key == InputEvent::Key::Escape) {
            stop = true;
          } else if (ie->key == InputEvent::Key::F) {
            vp.fitRect(graphBounds(tree), viewportSize);
          } else if (ie->key == InputEvent::Key::Space) {
            spaceHeld = true;
          }
          break;

        case InputEvent::Type::KeyUp:
          if (ie->key == InputEvent::Key::Space) {
            spaceHeld = false;
          }
          break;

        case InputEvent::Type::MouseDown:
          if (ie->button == InputEvent::Button::Middle || (ie->button == InputEvent::Button::Left && spaceHeld)) {
            panning = true;
            lastPan = ie->pos;
          }
          break;

        case InputEvent::Type::MouseUp:
          if (ie->button == InputEvent::Button::Middle || ie->button == InputEvent::Button::Left) {
            panning = false;
          }
          break;

        case InputEvent::Type::MouseMove:
          if (panning) {
            vp.pan = vp.pan + (ie->pos - lastPan);
            lastPan = ie->pos;
          }
          break;

        case InputEvent::Type::Wheel:
          {
            auto old = vp;
            vp.zoomAbout(ie->pos, std::pow(1.1, ie->wheel.y));
            if (vp.scale < ZOOM_LOWER_BOUND || vp.scale > ZOOM_UPPER_BOUND) {
              vp = old;
            }
            break;
          }

        case InputEvent::Type::Resize:
          viewportSize = ie->pos;
          break;
      }

      if (stop) {
        break;
      }
      redraw();
    }

    return 0;
  }

}  // namespace fluir::editor
