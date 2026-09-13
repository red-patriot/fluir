#include "editor/pages/splash.hpp"

#include <filesystem>
#include <memory>

#include <fmt/format.h>
#include <nfd.h>

#include "editor/pages/module.hpp"

namespace fluir::editor {
  namespace {
    constexpr double BUTTON_WIDTH = 120;
    constexpr double BUTTON_HEIGHT = 40;
  }  // namespace

  SplashPage::SplashPage(EditorContext& ctx, Renderer& renderer) :
    Page(ctx, renderer), open_{.label = "Open", .onClick = [this] { openFileDialog(); }} { }

  void SplashPage::onEvent(const InputEvent& event) {
    if (event.type == InputEvent::Type::MouseDown && event.button == InputEvent::Button::Left &&
        openRect_.contains(event.pos)) {
      open_.onClick();
    }
  }

  void SplashPage::onResize() {
    const Vec2 size = renderer_.outputSize();
    openRect_ = Rect{size.x / 2 - BUTTON_WIDTH / 2, size.y / 2 - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT};
  }

  void SplashPage::onDraw() { drawButton(renderer_, open_, openRect_, ctx_); }

  void SplashPage::openFileDialog() {
    nfdu8filteritem_t filter{"Fluir Program", "fl"};
    nfdu8char_t* outPath = nullptr;
    const nfdresult_t result = NFD_OpenDialogU8(&outPath, &filter, 1, nullptr);

    if (result == NFD_OKAY) {
      ctx_.program = std::filesystem::path(outPath);
      NFD_FreePathU8(outPath);
      next_ = std::make_unique<ModulePage>(ctx_, renderer_);
    } else if (result == NFD_ERROR) {
      fmt::print(stderr, "file dialog failed: {}\n", NFD_GetError());
    }
  }

}  // namespace fluir::editor
