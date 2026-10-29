#ifndef FLUIR_EDITOR_INPUT_HPP
#define FLUIR_EDITOR_INPUT_HPP

#include <optional>
#include <string>

#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** One input event for the tools. Retired with them in favour of Qt events. */
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

}  // namespace fluir::editor

#endif
