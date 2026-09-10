#include "editor/pages/splash.hpp"

#include <filesystem>
#include <memory>

#include <fmt/format.h>
#include <nfd.h>

#include "editor/core/interaction.hpp"
#include "editor/core/viewport.hpp"
#include "editor/pages/module.hpp"

namespace fluir::editor {

  SplashPage::SplashPage(EditorContext& ctx, Renderer& renderer) :
    ctx_(ctx),
    renderer_(renderer),
    openButton_(&root_.add(
      std::make_unique<ButtonActor>("Open", [this] { openFileDialog(); }, Rect{0, 0, kButtonWidth, kButtonHeight}))) {
    layer_.setRoot(root_);
    layer_.add(std::make_unique<ClickInteraction>());
  }

  int SplashPage::start() {
    layout();
    return 0;
  }

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

  void SplashPage::layout() {
    const Vec2 outputSize = renderer_.outputSize();
    openButton_->setBounds(
      Rect{outputSize.x / 2 - kButtonWidth / 2, outputSize.y / 2 - kButtonHeight / 2, kButtonWidth, kButtonHeight});
  }

  int SplashPage::update(const std::vector<InputEvent>& events) {
    for (const auto& ie : events) {
      if (ie.type == InputEvent::Type::Quit) {
        ctx_.running = false;
      } else if (ie.type == InputEvent::Type::Resize) {
        layout();
      } else {
        layer_.dispatch(ie, ctx_, renderer_.outputSize());
      }
    }
    return 0;
  }

  int SplashPage::draw() {
    renderer_.beginFrame();
    layer_.draw(renderer_, ctx_, Rect{0, 0, renderer_.outputSize().x, renderer_.outputSize().y});
    renderer_.endFrame();
    return 0;
  }

}  // namespace fluir::editor
