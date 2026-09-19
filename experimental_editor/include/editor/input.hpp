#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "editor/core/geometry.hpp"

namespace fluir::editor {

  struct InputEvent {
    enum class Type { Quit, KeyDown, KeyUp, MouseDown, MouseUp, MouseMove, Wheel, Resize, TextInput };
    enum class Key { Escape, F, Space, Delete, Return, Backspace, Left, Right, Home, End };
    enum class Button { Left, Middle, Right };

    Type type{};                   ///< The type of the event
    std::optional<Key> key;        ///< set for KeyDown / KeyUp
    std::optional<Button> button;  ///< set for MouseDown / MouseUp
    Vec2 pos;                      ///< mouse pos for mouse events
    Vec2 wheel;                    ///< scroll delta for Wheel
    std::string text;              ///< set for TextInput (UTF-8)
  };

  /** Map one SDL event to an InputEvent, or std::nullopt for events the viewer
   *  ignores (unmapped keys, other event types). Pure — no SDL state touched. */
  std::optional<InputEvent> translate(const SDL_Event& event);

  /** Reads batches of translated input events from SDL each tick. */
  class InputManager {
   public:
    /** Blocks up to `timeout` for the first event (returns an empty vector on
     *  timeout). */
    std::vector<InputEvent> read(std::chrono::milliseconds timeout);
  };

}  // namespace fluir::editor
