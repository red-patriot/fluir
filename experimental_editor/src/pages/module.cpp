#include "editor/pages/module.hpp"

#include <cmath>

#include <SDL3/SDL.h>

#include "compiler/utility/context.hpp"
#include "editor/core/loader.hpp"
#include "editor/core/render_graph.hpp"
#include "editor/input.hpp"

namespace fluir::editor {
  ModulePage::ModulePage(EditorContext& ctx, Renderer& renderer) : ctx_(ctx), renderer_(renderer) { }

  int ModulePage::run() {
    reset();
    fluir::Context cctx{
      .currentFile = *ctx_.program,  // TODO: Handle no program
      .ignoreVersionChecks = true,
    };

    const auto result = loadFile(cctx, *ctx_.program);

    if (!result.tree) {
      fmt::print(stderr, "parse failed: {}\n", ctx_.program->string());
      return 1;
    }
    tree_ = *result.tree;

    auto viewSize = renderer_.outputSize();
    view_.fitRect(graphBounds(ctx_, *tree_), viewSize);
    redraw();

    bool panning = false;
    bool spaceHeld = false;
    Vec2 lastPan;

    SDL_Event e;  // TODO: Abstract this out
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
            view_.fitRect(graphBounds(ctx_, *tree_), viewSize);
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
            view_.pan = view_.pan + (ie->pos - lastPan);
            lastPan = ie->pos;
          }
          break;

        case InputEvent::Type::Wheel:
          {
            auto old = view_;
            view_.zoomAbout(ie->pos, std::pow(ctx_.zoom.wheelStep, ie->wheel.y));
            if (view_.scale < ctx_.zoom.min || view_.scale > ctx_.zoom.max) {
              view_ = old;
            }
            break;
          }

        case InputEvent::Type::Resize:
          viewSize = renderer_.outputSize();
          break;
      }

      if (stop) {
        break;
      }
      redraw();
    }

    return 0;
  }

  void ModulePage::reset() {
    view_ = Viewport{};
    tree_.reset();
  }

  void ModulePage::redraw() {
    renderer_.beginFrame();

    renderGraph(ctx_, *tree_, view_, renderer_);
    renderer_.endFrame();
  };

}  // namespace fluir::editor
