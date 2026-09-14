#pragma once

#include "editor/tools/tool.hpp"

namespace fluir::editor {

  /** Left-press on a rail's type tag opens a menu of its types; a pick sets it. Never consumes. */
  class TypeTool : public Tool {
   public:
    bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) override;
  };

}  // namespace fluir::editor
