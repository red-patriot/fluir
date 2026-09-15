#pragma once

#include <functional>
#include <string>
#include <vector>

#include "editor/tools/tool.hpp"

namespace fluir::editor {

  /** One context-menu row: `onClick` runs on pick, after which the menu closes. Must not touch `EditorState::popup`. */
  struct MenuItem {
    std::string label;
    std::function<void(EditorState&)> onClick;
  };

  /** Items for a right-press on `hit` at `world`; empty when this provider has nothing for it. */
  using MenuProvider = std::function<std::vector<MenuItem>(const Box& hit, Vec2 world, const EditorState&)>;

  /** Right-press on a hit box opens the first provider's non-empty items at the cursor. Consumes only when it opens. */
  class ContextMenuTool : public Tool {
   public:
    explicit ContextMenuTool(std::vector<MenuProvider> providers = {});

    bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) override;

   private:
    std::vector<MenuProvider> providers_;
  };

}  // namespace fluir::editor
