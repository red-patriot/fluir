#ifndef FLUIR_EDITOR_PAGES_PAGE_HPP
#define FLUIR_EDITOR_PAGES_PAGE_HPP

#include <memory>
#include <vector>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/renderer.hpp"
#include "editor/input.hpp"

namespace fluir::editor {
  /** Owns the frame lifecycle every page shares: start, Quit/Resize handling and
   *  the beginFrame/endFrame bracket. Subclasses supply the hooks. */
  class Page {
   public:
    Page(EditorContext& ctx, Renderer& renderer) : ctx_(ctx), renderer_(renderer) { }
    virtual ~Page() = default;

    /** Initialize a page. Called when it is first displayed. */
    int start();
    /** Respond to events to update a page's state. */
    int update(const std::vector<InputEvent>& events);
    /** Draw this page's current state. */
    int draw();

    /** Returns a replacement page to transition to, or nullptr to stay on this one. */
    virtual std::unique_ptr<Page> next() { return nullptr; }

   protected:
    virtual int onStart() { return 0; }
    /** Every event but Quit and Resize. */
    virtual void onEvent(const InputEvent&) { }
    /** Re-lays-out chrome for the current output size. Runs after onStart and on Resize. */
    virtual void onResize() { }
    /** Draws inside the frame bracket. */
    virtual void onDraw() = 0;

    Rect outputRect() const;

    EditorContext& ctx_;
    Renderer& renderer_;
  };
}  // namespace fluir::editor

#endif
