#include "editor/core/interaction.hpp"

#include <memory>

#include <gtest/gtest.h>

#include "editor/actors/actor.hpp"
#include "editor/components/container_actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"

// These tests assert the *chain's routing contract*: who consumes an event,
// who captures the gesture that follows, and when capture is released.

namespace {

  using fluir::editor::Actor;
  using fluir::editor::ClickInteraction;
  using fluir::editor::ContainerActor;
  using fluir::editor::DragInteraction;
  using fluir::editor::EditorContext;
  using fluir::editor::InputEvent;
  using fluir::editor::InteractionChain;
  using fluir::editor::InteractionContext;
  using fluir::editor::PanZoomInteraction;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;

  // Claims a drag anywhere inside itself and records what it received.
  class DraggableActor : public Actor {
   public:
    explicit DraggableActor(Rect bounds) : Actor(bounds) { }

    bool onDragStart(const EditorContext&, Vec2) override {
      dragging_ = true;
      return true;
    }
    void onDrag(const EditorContext&, Vec2, Vec2 delta) override { dragged_ = dragged_ + delta; }
    void onDragEnd(const EditorContext&, Vec2) override { dragging_ = false; }
    void onDragCancel() override {
      dragging_ = false;
      ++cancels_;
    }
    void onClick(Vec2) override { ++clicks_; }

    bool dragging_ = false;
    Vec2 dragged_{};
    int clicks_ = 0;
    int cancels_ = 0;
  };

  // Never claims a drag: a press on it is a plain click.
  class InertActor : public Actor {
   public:
    explicit InertActor(Rect bounds) : Actor(bounds) { }

    void onClick(Vec2 pos) override {
      ++clicks_;
      lastClickPos_ = pos;
    }

    int clicks_ = 0;
    Vec2 lastClickPos_{};
  };

  struct Fixture {
    EditorContext editor;
    Viewport view;
    ContainerActor root{Rect{0, 0, 0, 0}, Actor::ClipChildren::No};
    InteractionChain chain;

    Fixture() {
      chain.add(std::make_unique<PanZoomInteraction>());
      chain.add(std::make_unique<DragInteraction>());
      chain.add(std::make_unique<ClickInteraction>());
    }

    InteractionContext context() { return InteractionContext{editor, view, root, Vec2{800, 600}}; }

    bool send(const InputEvent& event) {
      InteractionContext ctx = context();
      return chain.dispatch(event, ctx);
    }
  };

  InputEvent down(InputEvent::Button button, Vec2 pos) {
    return InputEvent{.type = InputEvent::Type::MouseDown, .button = button, .pos = pos};
  }
  InputEvent up(InputEvent::Button button, Vec2 pos) {
    return InputEvent{.type = InputEvent::Type::MouseUp, .button = button, .pos = pos};
  }
  InputEvent move(Vec2 pos) { return InputEvent{.type = InputEvent::Type::MouseMove, .pos = pos}; }
  InputEvent key(InputEvent::Type type, InputEvent::Key k) { return InputEvent{.type = type, .key = k}; }

}  // namespace

TEST(InteractionChain, DragCapturesTheGestureThatFollows) {
  Fixture f;
  auto& node = static_cast<DraggableActor&>(f.root.add(std::make_unique<DraggableActor>(Rect{0, 0, 50, 50})));

  ASSERT_TRUE(f.send(down(InputEvent::Button::Left, Vec2{10, 10})));
  ASSERT_TRUE(node.dragging_);

  EXPECT_TRUE(f.send(move(Vec2{30, 10})));
  EXPECT_EQ(node.dragged_, (Vec2{20, 0}));
  EXPECT_EQ(f.view.pan, (Vec2{0, 0})) << "a captured drag must not reach PanZoomInteraction";
}

TEST(InteractionChain, CaptureReleasesOnMouseUp) {
  Fixture f;
  auto& node = static_cast<DraggableActor&>(f.root.add(std::make_unique<DraggableActor>(Rect{0, 0, 50, 50})));

  ASSERT_TRUE(f.send(down(InputEvent::Button::Left, Vec2{10, 10})));
  ASSERT_TRUE(f.send(up(InputEvent::Button::Left, Vec2{10, 10})));
  EXPECT_FALSE(node.dragging_);

  // With capture released, a bare move is nobody's business again.
  EXPECT_FALSE(f.send(move(Vec2{200, 200})));
  EXPECT_EQ(node.dragged_, (Vec2{0, 0}));
}

TEST(InteractionChain, SpaceLeftPansEvenOverADraggableActor) {
  Fixture f;
  auto& node = static_cast<DraggableActor&>(f.root.add(std::make_unique<DraggableActor>(Rect{0, 0, 50, 50})));

  f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::Space));
  ASSERT_TRUE(f.send(down(InputEvent::Button::Left, Vec2{10, 10})));
  EXPECT_FALSE(node.dragging_) << "the pan gesture must win the press";

  EXPECT_TRUE(f.send(move(Vec2{30, 40})));
  EXPECT_EQ(f.view.pan, (Vec2{20, 30}));
}

TEST(InteractionChain, MiddleDragPansAndReleases) {
  Fixture f;
  f.root.add(std::make_unique<DraggableActor>(Rect{0, 0, 50, 50}));

  ASSERT_TRUE(f.send(down(InputEvent::Button::Middle, Vec2{10, 10})));
  ASSERT_TRUE(f.send(move(Vec2{15, 25})));
  EXPECT_EQ(f.view.pan, (Vec2{5, 15}));

  ASSERT_TRUE(f.send(up(InputEvent::Button::Middle, Vec2{15, 25})));
  EXPECT_FALSE(f.send(move(Vec2{100, 100})));
  EXPECT_EQ(f.view.pan, (Vec2{5, 15}));
}

TEST(InteractionChain, SpaceReleaseStopsPanClaimingLeftPresses) {
  Fixture f;
  auto& node = static_cast<DraggableActor&>(f.root.add(std::make_unique<DraggableActor>(Rect{0, 0, 50, 50})));

  f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::Space));
  f.send(key(InputEvent::Type::KeyUp, InputEvent::Key::Space));

  ASSERT_TRUE(f.send(down(InputEvent::Button::Left, Vec2{10, 10})));
  EXPECT_TRUE(node.dragging_);
}

TEST(InteractionChain, PressOnANonDraggableActorIsAClick) {
  Fixture f;
  auto& node = static_cast<InertActor&>(f.root.add(std::make_unique<InertActor>(Rect{10, 10, 50, 50})));

  EXPECT_TRUE(f.send(down(InputEvent::Button::Left, Vec2{20, 30})));
  EXPECT_EQ(node.clicks_, 1);
  EXPECT_EQ(node.lastClickPos_, (Vec2{20, 30}));

  // No capture: the move that follows reaches nobody.
  EXPECT_FALSE(f.send(move(Vec2{40, 40})));
}

TEST(InteractionChain, LeftClickOnEmptySpaceIsInert) {
  Fixture f;
  f.root.add(std::make_unique<InertActor>(Rect{10, 10, 50, 50}));

  EXPECT_FALSE(f.send(down(InputEvent::Button::Left, Vec2{500, 500})));
  EXPECT_EQ(f.view.pan, (Vec2{0, 0}));
}

TEST(InteractionChain, WheelZoomsAndClampsToTheContextRange) {
  Fixture f;

  ASSERT_TRUE(f.send(InputEvent{.type = InputEvent::Type::Wheel, .pos = {0, 0}, .wheel = {0, 1}}));
  EXPECT_GT(f.view.scale, 1.0);

  const double zoomedIn = f.view.scale;
  for (int i = 0; i < 100; ++i) {
    f.send(InputEvent{.type = InputEvent::Type::Wheel, .pos = {0, 0}, .wheel = {0, 1}});
  }
  EXPECT_LE(f.view.scale, f.editor.zoom.max);
  EXPECT_GE(f.view.scale, zoomedIn);
}

TEST(InteractionChain, ResetDropsCaptureAndActorPointers) {
  Fixture f;
  auto& node = static_cast<DraggableActor&>(f.root.add(std::make_unique<DraggableActor>(Rect{0, 0, 50, 50})));

  ASSERT_TRUE(f.send(down(InputEvent::Button::Left, Vec2{10, 10})));
  f.chain.reset();

  EXPECT_FALSE(f.send(move(Vec2{30, 10})));
  EXPECT_EQ(node.dragged_, (Vec2{0, 0}));
}

TEST(DragInteraction, ResetCancelsTheActorsDrag) {
  Fixture f;
  auto& node = static_cast<DraggableActor&>(f.root.add(std::make_unique<DraggableActor>(Rect{0, 0, 50, 50})));

  ASSERT_TRUE(f.send(down(InputEvent::Button::Left, Vec2{10, 10})));
  ASSERT_TRUE(f.send(move(Vec2{30, 40})));
  f.chain.reset();

  EXPECT_EQ(node.cancels_, 1) << "a gesture dropped without a MouseUp must reach the actor";
  EXPECT_FALSE(node.dragging_);
}
