#include "editor/pages/splash.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <utility>

#include <fmt/format.h>
#include <nfd.h>

#include "editor/core/module_editor.hpp"
#include "editor/pages/module.hpp"

namespace fluir::editor {
  namespace {
    constexpr double BUTTON_WIDTH = 120;
    constexpr double BUTTON_HEIGHT = 40;
  }  // namespace

  SplashPage::SplashPage(EditorContext& ctx, Renderer& renderer) :
    Page(ctx, renderer),
    new_{.label = "New", .onClick = [this] { newFile(); }},
    open_{.label = "Open", .onClick = [this] { openFileDialog(); }} { }

  void SplashPage::onEvent(const InputEvent& event) {
    if (event.type == InputEvent::Type::MouseDown && event.button == InputEvent::Button::Left) {
      if (openRect_.contains(event.pos)) {
        open_.onClick();
      }
      if (newRect_.contains(event.pos)) {
        new_.onClick();
      }
    }
  }

  void SplashPage::onResize() {
    const Vec2 size = renderer_.outputSize();
    newRect_ = Rect{
      .x = size.x / 2 - BUTTON_WIDTH / 2, .y = size.y * 0.4 - BUTTON_HEIGHT / 2, .w = BUTTON_WIDTH, .h = BUTTON_HEIGHT};
    openRect_ = Rect{
      .x = size.x / 2 - BUTTON_WIDTH / 2, .y = size.y * 0.6 - BUTTON_HEIGHT / 2, .w = BUTTON_WIDTH, .h = BUTTON_HEIGHT};
  }

  void SplashPage::onDraw() {
    drawButton(renderer_, new_, newRect_, ctx_);
    drawButton(renderer_, open_, openRect_, ctx_);
  }

  void SplashPage::newFile() { next_ = std::make_unique<ModulePage>(ctx_, renderer_, newModule()); }

  void SplashPage::openFileDialog() {
    nfdu8filteritem_t filter{"Fluir Program", "fl"};
    nfdu8char_t* outPath = nullptr;
    const nfdresult_t result = NFD_OpenDialogU8(&outPath, &filter, 1, nullptr);

    if (result == NFD_OKAY) {
      const std::filesystem::path program(outPath);
      NFD_FreePathU8(outPath);
      // A file that fails to load leaves us here to pick another.
      if (std::optional<ModuleEditor> editor = openModule(program)) {
        next_ = std::make_unique<ModulePage>(ctx_, renderer_, std::move(*editor));
      } else {
        fmt::print(stderr, "parse failed: {}\n", program.string());
      }
    } else if (result == NFD_ERROR) {
      fmt::print(stderr, "file dialog failed: {}\n", NFD_GetError());
    }
  }

}  // namespace fluir::editor
