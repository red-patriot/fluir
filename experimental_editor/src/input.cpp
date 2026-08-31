#include "../include/editor/input.hpp"

namespace fluir::editor {

  namespace {

    std::optional<InputEvent::Key> mapScancode(SDL_Scancode scancode) {
      switch (scancode) {
        case SDL_SCANCODE_ESCAPE:
          return InputEvent::Key::Escape;
        case SDL_SCANCODE_F:
          return InputEvent::Key::F;
        case SDL_SCANCODE_SPACE:
          return InputEvent::Key::Space;
        default:
          return std::nullopt;
      }
    }

    std::optional<InputEvent::Button> mapButton(Uint8 button) {
      switch (button) {
        case SDL_BUTTON_LEFT:
          return InputEvent::Button::Left;
        case SDL_BUTTON_MIDDLE:
          return InputEvent::Button::Middle;
        case SDL_BUTTON_RIGHT:
          return InputEvent::Button::Right;
        default:
          return std::nullopt;
      }
    }

  }  // namespace

  std::optional<InputEvent> translate(const SDL_Event& event) {
    switch (event.type) {
      case SDL_EVENT_QUIT:
        {
          InputEvent out{};
          out.type = InputEvent::Type::Quit;
          return out;
        }

      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP:
        {
          const auto key = mapScancode(event.key.scancode);
          if (!key) {
            return std::nullopt;
          }
          InputEvent out{};
          out.type = event.type == SDL_EVENT_KEY_DOWN ? InputEvent::Type::KeyDown : InputEvent::Type::KeyUp;
          out.key = key;
          return out;
        }

      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      case SDL_EVENT_MOUSE_BUTTON_UP:
        {
          const auto button = mapButton(event.button.button);
          if (!button) {
            return std::nullopt;
          }
          InputEvent out{};
          out.type =
            event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? InputEvent::Type::MouseDown : InputEvent::Type::MouseUp;
          out.button = button;
          out.pos = {static_cast<double>(event.button.x), static_cast<double>(event.button.y)};
          return out;
        }

      case SDL_EVENT_MOUSE_MOTION:
        {
          InputEvent out{};
          out.type = InputEvent::Type::MouseMove;
          out.pos = {static_cast<double>(event.motion.x), static_cast<double>(event.motion.y)};
          return out;
        }

      case SDL_EVENT_MOUSE_WHEEL:
        {
          InputEvent out{};
          out.type = InputEvent::Type::Wheel;
          out.pos = {static_cast<double>(event.wheel.mouse_x), static_cast<double>(event.wheel.mouse_y)};
          out.wheel = {static_cast<double>(event.wheel.x), static_cast<double>(event.wheel.y)};
          return out;
        }

      case SDL_EVENT_WINDOW_RESIZED:
      case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        {
          InputEvent out{};
          out.type = InputEvent::Type::Resize;
          out.pos = {static_cast<double>(event.window.data1), static_cast<double>(event.window.data2)};
          return out;
        }

      default:
        return std::nullopt;
    }
  }

}  // namespace fluir::editor
