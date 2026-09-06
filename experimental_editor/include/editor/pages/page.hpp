#ifndef FLUIR_EDITOR_PAGES_PAGE_HPP
#define FLUIR_EDITOR_PAGES_PAGE_HPP

#include <memory>
#include <vector>

#include "editor/core/renderer.hpp"
#include "editor/input.hpp"

namespace fluir::editor {
  class Page {
   public:
    virtual ~Page() = default;

    /** Initialize a page. Called when it is first displayed */
    virtual int start() = 0;
    /** Respond to events to update a page's state */
    virtual int update(const std::vector<InputEvent>& events) = 0;
    /** Draw this page's current state */
    virtual int draw() = 0;

    /** Returns a replacement page to transition to, or nullptr to stay on this one. */
    virtual std::unique_ptr<Page> next() { return nullptr; }
  };
}  // namespace fluir::editor

#endif
