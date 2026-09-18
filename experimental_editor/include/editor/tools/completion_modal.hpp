#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/components/menu.hpp"
#include "editor/components/text_field.hpp"
#include "editor/core/intelligence.hpp"
#include "editor/core/renderer.hpp"
#include "editor/tools/popup.hpp"

namespace fluir::editor {

  /** A centered modal listing completions; Escape or a press outside closes. A left press on a row adds it. A search
   * bar at the top filters the rows by a case-insensitive substring of their labels. */
  class CompletionModal : public Popup {
   public:
    /** `bounds` in screen px; the modal is half its width, as tall as its search bar and rows (at most 10 and the
     * bounds, then wheel-scrolled), centered. Rows fit `text`'s measured line height (null falls back to GLYPH_PX).
     * Picks land at world units `where` (z is the parent's) inside `body`. */
    CompletionModal(
      std::vector<Completion> completions, Rect bounds, Renderer* text, Coordinate where = {}, FullID body = {});

    bool onEvent(const InputEvent& event, EditorState& state) override;
    void draw(Renderer& renderer, const EditorContext& ctx) const override;

    const std::vector<std::string>& labels() const { return labels_; }
    Rect frame() const { return layout_.frame; }
    Rect searchBar() const { return search_; }

   private:
    std::vector<Completion> completions_;
    std::vector<std::string> labels_;
    std::vector<std::size_t> visible_;  // indices into completions_ that pass the filter
    TextField query_{"", 0};
    MenuLayout layout_;  // rows unscrolled
    Rect bounds_;
    Rect search_;
    Rect rows_;  // the region the rows scroll inside, below the search bar
    double rowH_ = 0;
    double rowStep_ = 0;
    double maxScroll_ = 0;
    double scroll_ = 0;  // px the rows are shifted up
    Coordinate where_;
    FullID body_;
    std::optional<std::size_t> hovered_;

    std::optional<std::size_t> rowAt(Vec2 screen) const;
    void relayout();
    void applyFilter();

    void selectVisible(size_t idx, EditorState& state);
  };

}  // namespace fluir::editor
