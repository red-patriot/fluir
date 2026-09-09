#ifndef FLUIR_EDITOR_PAGES_SPLASH_HPP
#define FLUIR_EDITOR_PAGES_SPLASH_HPP

#include <memory>
#include <vector>

#include "editor/components/button_actor.hpp"
#include "editor/components/container_actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/layer.hpp"
#include "editor/core/renderer.hpp"
#include "editor/input.hpp"
#include "editor/pages/page.hpp"

namespace fluir::editor {
  /** Landing page shown before a program is opened. */
  class SplashPage : public Page {
   public:
    SplashPage(EditorContext& ctx, Renderer& renderer);

    int start() override;
    int update(const std::vector<InputEvent>& events) override;
    int draw() override;
    std::unique_ptr<Page> next() override { return std::move(next_); }

    // Test-only observability: lets tests locate/click the Open button
    // without a second parallel path through SplashPage's API.
    const ButtonActor& openButton() const { return *openButton_; }

   private:
    static constexpr int kButtonWidth = 120;
    static constexpr int kButtonHeight = 40;

    EditorContext& ctx_;
    Renderer& renderer_;
    // The page spans the whole output, so it neither offsets nor clips its button.
    ContainerActor root_{Rect{0, 0, 0, 0}, Actor::ClipChildren::No};
    ButtonActor* openButton_;
    Layer layer_;
    std::unique_ptr<Page> next_;

    void layout();
    void openFileDialog();
  };
}  // namespace fluir::editor

#endif
