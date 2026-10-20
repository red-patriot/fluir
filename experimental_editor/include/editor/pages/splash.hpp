#ifndef FLUIR_EDITOR_PAGES_SPLASH_HPP
#define FLUIR_EDITOR_PAGES_SPLASH_HPP

#include <memory>

#include "editor/components/button.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/renderer.hpp"
#include "editor/pages/page.hpp"

namespace fluir::editor {
  /** Landing page shown before a program is opened. */
  class SplashPage : public Page {
   public:
    SplashPage(EditorContext& ctx, Renderer& renderer);

    std::unique_ptr<Page> next() override { return std::move(next_); }

   protected:
    void onEvent(const InputEvent& event) override;
    void onResize() override;
    void onDraw() override;

   private:
    std::vector<Button> buttons_; /**< The list of buttons from top-down */
    std::unique_ptr<Page> next_;

    void newFile();
    void openFileDialog();
  };
}  // namespace fluir::editor

#endif
