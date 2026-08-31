#pragma once

#include <optional>

#include <SDL3/SDL.h>

#include "editor/core/geometry.hpp"

namespace fluir::editor {

  struct InputEvent {
    enum class Type { Quit, KeyDown, KeyUp, MouseDown, MouseUp, MouseMove, Wheel, Resize };
    enum class Key { Escape, F, Space };
    enum class Button { Left, Middle, Right };

    Type type{};
    std::optional<Key> key;        ///< set for KeyDown / KeyUp
    std::optional<Button> button;  ///< set for MouseDown / MouseUp
    Vec2 pos;                      ///< mouse pos for mouse events; new (w,h) for Resize
    Vec2 wheel;                    ///< scroll delta for Wheel
  };

  /** Map one SDL event to an InputEvent, or std::nullopt for events the viewer
   *  ignores (unmapped keys, other event types). Pure — no SDL state touched. */
  std::optional<InputEvent> translate(const SDL_Event& event);

}  // namespace fluir::editor
