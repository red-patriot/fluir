#pragma once

#include <string>
#include <vector>

#include "editor/input.hpp"
#include "editor/tools/tool.hpp"
#include "editor/view/graph_layout.hpp"
#include "fixture_loader.hpp"

namespace testutil {

  inline fluir::editor::InputEvent down(
    fluir::editor::Vec2 pos, fluir::editor::InputEvent::Button button = fluir::editor::InputEvent::Button::Left) {
    return {.type = fluir::editor::InputEvent::Type::MouseDown, .button = button, .pos = pos};
  }

  inline fluir::editor::InputEvent up(
    fluir::editor::Vec2 pos, fluir::editor::InputEvent::Button button = fluir::editor::InputEvent::Button::Left) {
    return {.type = fluir::editor::InputEvent::Type::MouseUp, .button = button, .pos = pos};
  }

  inline fluir::editor::InputEvent move(fluir::editor::Vec2 pos) {
    return {.type = fluir::editor::InputEvent::Type::MouseMove, .pos = pos};
  }

  inline fluir::editor::InputEvent key(fluir::editor::InputEvent::Key k) {
    return {.type = fluir::editor::InputEvent::Type::KeyDown, .key = k};
  }

  inline fluir::editor::InputEvent text(std::string s) {
    return {.type = fluir::editor::InputEvent::Type::TextInput, .text = std::move(s)};
  }

  /** Sends `event` to `tool` against a fresh layout of the state's tree, as the page does. */
  inline bool send(fluir::editor::Tool& tool,
                   fluir::editor::EditorState& state,
                   const fluir::editor::InputEvent& event) {
    const std::vector<fluir::editor::Box> boxes = fluir::editor::layoutGraph(state.editor.tree(), state.ctx.layout);
    return tool.onEvent(event, state, boxes);
  }

  /** Loads fixture `relPath` into `state`'s editor. */
  inline void loadInto(fluir::editor::EditorState& state, const std::string& relPath) {
    const Loaded loaded = loadFixture(relPath);
    ASSERT_TRUE(loaded.result.tree.has_value());
    state.editor.load(*loaded.result.tree);
  }

}  // namespace testutil
