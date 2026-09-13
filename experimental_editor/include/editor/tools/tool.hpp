#pragma once

#include <memory>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/module_editor.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"
#include "editor/view/graph_layout.hpp"

namespace fluir::editor {

  class Renderer;

  /** Everything a tool may read or change. */
  struct EditorState {
    const EditorContext& ctx;
    ModuleEditor editor;
    std::optional<FullID> selection;
    Viewport view;
    /** Wrapped-text layout queries; null puts the caret at the end. */
    Renderer* text = nullptr;
  };

  /** One way of handling graph input. */
  class Tool {
   public:
    virtual ~Tool() = default;

    /** True consumes. A consumed MouseDown also captures while `capturing()`. */
    virtual bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) = 0;

    virtual bool capturing() const { return false; }

    /** Abandons any live gesture or draft, leaving the tree as it was. */
    virtual void cancel(EditorState&) { }

    /** Draws over the graph. */
    virtual void draw(const Subview&, const EditorState&, std::span<const Box>) const { }
  };

  /** Tools in priority order: the first to consume wins; a capturing tool gets everything. */
  class ToolChain {
   public:
    void add(std::unique_ptr<Tool> tool) { tools_.push_back(std::move(tool)); }
    bool dispatch(const InputEvent& event, EditorState& state, std::span<const Box> boxes);
    void cancel(EditorState& state);
    void draw(const Subview& view, const EditorState& state, std::span<const Box> boxes) const;

   private:
    std::vector<std::unique_ptr<Tool>> tools_;
    Tool* captured_ = nullptr;
  };

}  // namespace fluir::editor
