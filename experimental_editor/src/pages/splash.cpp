#include "editor/pages/splash.hpp"

#include "editor/core/viewport.hpp"

namespace fluir::editor {

  SplashPage::SplashPage(EditorContext& ctx, Renderer& renderer) :
    ctx_(ctx),
    renderer_(renderer),
    openButton_(
      "Open",
      [] {},
      Rect{renderer.outputSize().x / 2 - kButtonWidth / 2,
           renderer.outputSize().y / 2 - kButtonHeight / 2,
           kButtonWidth,
           kButtonHeight}) { }

  int SplashPage::start() { return 0; }

  void SplashPage::layout() {
    const Vec2 outputSize = renderer_.outputSize();
    openButton_.setBounds(
      Rect{outputSize.x / 2 - kButtonWidth / 2, outputSize.y / 2 - kButtonHeight / 2, kButtonWidth, kButtonHeight});
  }

  int SplashPage::update(const std::vector<InputEvent>& events) {
    for (const auto& ie : events) {
      if (ie.type == InputEvent::Type::Quit) {
        ctx_.running = false;
      } else if (ie.type == InputEvent::Type::MouseDown && ie.button == InputEvent::Button::Left) {
        if (openButton_.bounds().contains(ie.pos)) {
          openButton_.onClick(ie.pos);
        }
      }
    }
    return 0;
  }

  int SplashPage::draw() {
    layout();
    renderer_.beginFrame();

    const Viewport identity;
    const Subview view{identity, Rect{0, 0, renderer_.outputSize().x, renderer_.outputSize().y}, renderer_};
    openButton_.draw(view, ctx_);

    renderer_.endFrame();
    return 0;
  }

}  // namespace fluir::editor
