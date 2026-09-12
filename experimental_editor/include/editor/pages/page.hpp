#ifndef FLUIR_EDITOR_PAGES_PAGE_HPP
#define FLUIR_EDITOR_PAGES_PAGE_HPP

#include <memory>
#include <vector>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/layer.hpp"
#include "editor/core/renderer.hpp"
#include "editor/input.hpp"

namespace fluir::editor {
  /** Owns the frame lifecycle every page shares: start, event dispatch and the
   *  beginFrame/endFrame bracket. Subclasses supply their layers and hooks. */
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
    /** Topmost first: dispatch walks this order, draw walks it reversed. */
    virtual std::vector<Layer*> layers() = 0;

    virtual int onStart() { return 0; }
    /** Page-specific handling, for events no layer consumed. */
    virtual bool onAppEvent(const InputEvent&) { return false; }
    /** Re-lays-out chrome for the current output size. Runs after onStart and on Resize. */
    virtual void onResize() { }
    /** Runs once after each batch of events, e.g. to re-layout the scene. */
    virtual void afterUpdate() { }

    EditorContext& ctx_;
    Renderer& renderer_;

    Rect outputRect() const;
  };
}  // namespace fluir::editor

#endif
