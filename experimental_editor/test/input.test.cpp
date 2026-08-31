#include "editor/input.hpp"

#include <gtest/gtest.h>
#include <SDL3/SDL.h>

namespace {

  using fluir::editor::InputEvent;
  using fluir::editor::translate;

  TEST(TranslateInput, Quit) {
    SDL_Event e{};
    e.type = SDL_EVENT_QUIT;

    const auto ie = translate(e);
    ASSERT_TRUE(ie.has_value());
    EXPECT_EQ(ie->type, InputEvent::Type::Quit);
  }

  TEST(TranslateInput, KeyDownEscape) {
    SDL_Event e{};
    e.type = SDL_EVENT_KEY_DOWN;
    e.key.scancode = SDL_SCANCODE_ESCAPE;

    const auto ie = translate(e);
    ASSERT_TRUE(ie.has_value());
    EXPECT_EQ(ie->type, InputEvent::Type::KeyDown);
    ASSERT_TRUE(ie->key.has_value());
    EXPECT_EQ(*ie->key, InputEvent::Key::Escape);
  }

  TEST(TranslateInput, KeyDownF) {
    SDL_Event e{};
    e.type = SDL_EVENT_KEY_DOWN;
    e.key.scancode = SDL_SCANCODE_F;

    const auto ie = translate(e);
    ASSERT_TRUE(ie.has_value());
    EXPECT_EQ(ie->type, InputEvent::Type::KeyDown);
    ASSERT_TRUE(ie->key.has_value());
    EXPECT_EQ(*ie->key, InputEvent::Key::F);
  }

  TEST(TranslateInput, KeyDownSpace) {
    SDL_Event e{};
    e.type = SDL_EVENT_KEY_DOWN;
    e.key.scancode = SDL_SCANCODE_SPACE;

    const auto ie = translate(e);
    ASSERT_TRUE(ie.has_value());
    EXPECT_EQ(ie->type, InputEvent::Type::KeyDown);
    ASSERT_TRUE(ie->key.has_value());
    EXPECT_EQ(*ie->key, InputEvent::Key::Space);
  }

  TEST(TranslateInput, KeyUpSpace) {
    SDL_Event e{};
    e.type = SDL_EVENT_KEY_UP;
    e.key.scancode = SDL_SCANCODE_SPACE;

    const auto ie = translate(e);
    ASSERT_TRUE(ie.has_value());
    EXPECT_EQ(ie->type, InputEvent::Type::KeyUp);
    ASSERT_TRUE(ie->key.has_value());
    EXPECT_EQ(*ie->key, InputEvent::Key::Space);
  }

  TEST(TranslateInput, KeyDownUnmappedIsIgnored) {
    SDL_Event e{};
    e.type = SDL_EVENT_KEY_DOWN;
    e.key.scancode = SDL_SCANCODE_A;

    EXPECT_FALSE(translate(e).has_value());
  }

  TEST(TranslateInput, MouseButtonDownMiddle) {
    SDL_Event e{};
    e.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    e.button.button = SDL_BUTTON_MIDDLE;
    e.button.x = 12.0F;
    e.button.y = 34.0F;

    const auto ie = translate(e);
    ASSERT_TRUE(ie.has_value());
    EXPECT_EQ(ie->type, InputEvent::Type::MouseDown);
    ASSERT_TRUE(ie->button.has_value());
    EXPECT_EQ(*ie->button, InputEvent::Button::Middle);
    EXPECT_NEAR(ie->pos.x, 12.0, 1e-6);
    EXPECT_NEAR(ie->pos.y, 34.0, 1e-6);
  }

  TEST(TranslateInput, MouseButtonUnmappedIsIgnored) {
    SDL_Event e{};
    e.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    e.button.button = SDL_BUTTON_X1;

    EXPECT_FALSE(translate(e).has_value());
  }

  TEST(TranslateInput, MouseMotion) {
    SDL_Event e{};
    e.type = SDL_EVENT_MOUSE_MOTION;
    e.motion.x = 5.0F;
    e.motion.y = 6.0F;

    const auto ie = translate(e);
    ASSERT_TRUE(ie.has_value());
    EXPECT_EQ(ie->type, InputEvent::Type::MouseMove);
    EXPECT_NEAR(ie->pos.x, 5.0, 1e-6);
    EXPECT_NEAR(ie->pos.y, 6.0, 1e-6);
  }

  TEST(TranslateInput, MouseWheel) {
    SDL_Event e{};
    e.type = SDL_EVENT_MOUSE_WHEEL;
    e.wheel.x = 0.0F;
    e.wheel.y = 2.0F;
    e.wheel.mouse_x = 7.0F;
    e.wheel.mouse_y = 8.0F;

    const auto ie = translate(e);
    ASSERT_TRUE(ie.has_value());
    EXPECT_EQ(ie->type, InputEvent::Type::Wheel);
    EXPECT_NEAR(ie->wheel.y, 2.0, 1e-6);
    EXPECT_NEAR(ie->pos.x, 7.0, 1e-6);
    EXPECT_NEAR(ie->pos.y, 8.0, 1e-6);
  }

  TEST(TranslateInput, WindowResized) {
    SDL_Event e{};
    e.type = SDL_EVENT_WINDOW_RESIZED;
    e.window.data1 = 1024;
    e.window.data2 = 768;

    const auto ie = translate(e);
    ASSERT_TRUE(ie.has_value());
    EXPECT_EQ(ie->type, InputEvent::Type::Resize);
    EXPECT_NEAR(ie->pos.x, 1024.0, 1e-6);
    EXPECT_NEAR(ie->pos.y, 768.0, 1e-6);
  }

  TEST(TranslateInput, WindowPixelSizeChanged) {
    SDL_Event e{};
    e.type = SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;
    e.window.data1 = 800;
    e.window.data2 = 600;

    const auto ie = translate(e);
    ASSERT_TRUE(ie.has_value());
    EXPECT_EQ(ie->type, InputEvent::Type::Resize);
    EXPECT_NEAR(ie->pos.x, 800.0, 1e-6);
    EXPECT_NEAR(ie->pos.y, 600.0, 1e-6);
  }

  TEST(TranslateInput, UnmappedEventIsIgnored) {
    SDL_Event e{};
    e.type = SDL_EVENT_FINGER_DOWN;

    EXPECT_FALSE(translate(e).has_value());
  }

}  // namespace
