#include "editor/pages/module.hpp"

#include <cmath>

#include "compiler/utility/context.hpp"
#include "editor/core/loader.hpp"
#include "editor/core/render_graph.hpp"

namespace fluir::editor {
  ModulePage::ModulePage(EditorContext& ctx, Renderer& renderer) : ctx_(ctx), renderer_(renderer) { }

  int ModulePage::start() {
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
    scene_.build(ctx_, *tree_);

    view_.fitRect(graphBounds(ctx_, *tree_), renderer_.outputSize());
    return 0;
  }

  int ModulePage::update(const std::vector<InputEvent>& events) {
    for (const auto& ie : events) {
      switch (ie.type) {
        case InputEvent::Type::Quit:
          ctx_.running = false;
          break;

        case InputEvent::Type::KeyDown:
          if (ie.key == InputEvent::Key::Escape) {
            ctx_.running = false;
          } else if (ie.key == InputEvent::Key::F) {
            view_.fitRect(graphBounds(ctx_, *tree_), renderer_.outputSize());
          } else if (ie.key == InputEvent::Key::Space) {
            spaceHeld_ = true;
          }
          break;

        case InputEvent::Type::KeyUp:
          if (ie.key == InputEvent::Key::Space) {
            spaceHeld_ = false;
          }
          break;

        case InputEvent::Type::MouseDown:
          if (ie.button == InputEvent::Button::Middle || (ie.button == InputEvent::Button::Left && spaceHeld_)) {
            panning_ = true;
            lastPan_ = ie.pos;
          } else if (ie.button == InputEvent::Button::Left) {
            if (Actor* hit = scene_.topmostAt(view_.screenToWorld(ie.pos))) {
              hit->onClick(view_.screenToWorld(ie.pos));
            }
          }
          break;

        case InputEvent::Type::MouseUp:
          if (ie.button == InputEvent::Button::Middle || ie.button == InputEvent::Button::Left) {
            panning_ = false;
          }
          break;

        case InputEvent::Type::MouseMove:
          if (panning_) {
            view_.pan = view_.pan + (ie.pos - lastPan_);
            lastPan_ = ie.pos;
          }
          break;

        case InputEvent::Type::Wheel:
          {
            auto old = view_;
            view_.zoomAbout(ie.pos, std::pow(ctx_.zoom.wheelStep, ie.wheel.y));
            if (view_.scale < ctx_.zoom.min || view_.scale > ctx_.zoom.max) {
              view_ = old;
            }
            break;
          }

        case InputEvent::Type::Resize:
          break;
      }
    }

    return 0;
  }

  int ModulePage::write() {
    renderer_.beginFrame();

    renderGraph(ctx_, *tree_, view_, renderer_);
    renderer_.endFrame();
    return 0;
  }

  void ModulePage::reset() {
    view_ = Viewport{};
    tree_.reset();
    scene_.clear();
    panning_ = false;
    spaceHeld_ = false;
    lastPan_ = Vec2{};
  }

}  // namespace fluir::editor
