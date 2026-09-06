#ifndef FLUIR_EDITOR_PAGES_SPLASH_HPP
#define FLUIR_EDITOR_PAGES_SPLASH_HPP

#include <memory>
#include <vector>

#include "editor/components/button_actor.hpp"
#include "editor/core/editor_context.hpp"
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

    static constexpr int kWidth = 480;
    static constexpr int kHeight = 320;

    // Test-only observability: lets tests locate/click the Open button
    // without a second parallel path through SplashPage's API.
    const ButtonActor& openButton() const { return openButton_; }

   private:
    static constexpr int kButtonWidth = 120;
    static constexpr int kButtonHeight = 40;

    EditorContext& ctx_;
    Renderer& renderer_;
    ButtonActor openButton_;
    std::unique_ptr<Page> next_;

    // Recomputes the centered button bounds for the current output size --
    // mirrors HeaderBar::layout(), since the window is resizable.
    void layout();

    // Pops a native "open file" dialog filtered to .fl programs. On a chosen
    // file, sets ctx_.program and stages a ModulePage in next_; cancelling
    // leaves next_ null so the app stays on the splash screen.
    void openFileDialog();
  };
}  // namespace fluir::editor

#endif
