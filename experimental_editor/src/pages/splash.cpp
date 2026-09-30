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
    buttons_{Button{ButtonOptions{.label = "New", .onClick = [this]() { newFile(); }}},
             Button{ButtonOptions{.label = "Open", .onClick = [this]() { openFileDialog(); }}}} { }

  void SplashPage::onEvent(const InputEvent& event) {
    for (auto& button : buttons_) {
      if (button.handle(event)) {
        break;
      }
    }
  }

  void SplashPage::onResize() {
    const Vec2 size = renderer_.outputSize();
    const double inc = 1.2 * BUTTON_HEIGHT;
    const auto count = buttons_.size();
    Rect buttonRect{.x = size.x / 2, .y = size.y / 2 - (inc * count / 2), .w = BUTTON_WIDTH, .h = BUTTON_HEIGHT};

    for (auto& button : buttons_) {
      button.place(buttonRect);
      buttonRect.y += inc;
    }
  }

  void SplashPage::onDraw() {
    std::ranges::for_each(buttons_, [&](auto& button) { button.draw(renderer_, ctx_.theme); });
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
