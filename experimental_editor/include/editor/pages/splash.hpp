#ifndef FLUIR_EDITOR_PAGES_SPLASH_HPP
#define FLUIR_EDITOR_PAGES_SPLASH_HPP

#include <memory>

#include "editor/core/editor_context.hpp"
#include "editor/core/renderer.hpp"
#include "editor/pages/page.hpp"
#include "editor/view/toolbar.hpp"

namespace fluir::editor {
  /** Landing page shown before a program is opened. */
  class SplashPage : public Page {
   public:
    SplashPage(EditorContext& ctx, Renderer& renderer);

    std::unique_ptr<Page> next() override { return std::move(next_); }

    // Test-only observability: where the Open button sits.
    Rect openButtonRect() const { return openRect_; }

   protected:
    void onEvent(const InputEvent& event) override;
    void onResize() override;
    void onDraw() override;

   private:
    Button new_;  /**< Opens a new blank module */
    Button open_; /**< Opens an existing module */
    Rect newRect_;
    Rect openRect_;
    std::unique_ptr<Page> next_;

    void newFile();
    void openFileDialog();
  };
}  // namespace fluir::editor

#endif
