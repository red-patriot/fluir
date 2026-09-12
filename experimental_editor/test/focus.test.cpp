#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "editor/actors/actor.hpp"
#include "editor/components/container_actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/interaction.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"

// These tests assert the *focus contract*: which actor a press hands the
// keyboard to, when it is taken away again, and that claiming focus never
// costs the actor the selection/drag/click dispatch it would otherwise get.

namespace {

  using fluir::editor::Actor;
  using fluir::editor::ClickInteraction;
  using fluir::editor::ContainerActor;
  using fluir::editor::DragInteraction;
  using fluir::editor::EditorContext;
  using fluir::editor::FocusInteraction;
  using fluir::editor::InputEvent;
  using fluir::editor::InteractionChain;
  using fluir::editor::InteractionContext;
  using fluir::editor::PanZoomInteraction;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;

  // Claims focus anywhere inside itself and records what it was handed.
  class FocusableActor : public Actor {
   public:
    explicit FocusableActor(Rect bounds) : Actor(bounds) { }

    bool onFocus(const EditorContext&, Vec2 position) override {
      lastFocusPos_ = position;
      focused_ = claimsFocus_;
      return claimsFocus_;
    }
    void onBlur() override { focused_ = false; }
    bool onKey(const EditorContext&, InputEvent::Key key) override {
      keys_.push_back(key);
      return true;
    }
    bool onTextInput(const EditorContext&, std::string_view text) override {
      typed_ += text;
      return true;
    }
    void onClick(Vec2) override { ++clicks_; }
    bool onDragStart(const EditorContext&, Vec2) override {
      dragging_ = claimsDrag_;
      return claimsDrag_;
    }
    void onDragEnd(const EditorContext&, Vec2) override { dragging_ = false; }
    void onDragCancel() override { dragging_ = false; }

    bool claimsFocus_ = true;
    bool claimsDrag_ = false; /**< a drag captures the chain, so keys stop flowing */
    bool focused_ = false;
    bool dragging_ = false;
    Vec2 lastFocusPos_{};
    std::vector<InputEvent::Key> keys_;
    std::string typed_;
    int clicks_ = 0;
  };

  // Never claims focus: a press on it is a plain click.
  class PlainActor : public Actor {
   public:
    explicit PlainActor(Rect bounds) : Actor(bounds) { }

    void onClick(Vec2) override { ++clicks_; }

    int clicks_ = 0;
  };

  struct Fixture {
    EditorContext editor;
    Viewport view;
    ContainerActor root{Rect{0, 0, 0, 0}, Actor::ClipChildren::No};
    InteractionChain chain;

    Fixture() {
      chain.add(std::make_unique<FocusInteraction>());
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
  InputEvent key(InputEvent::Type type, InputEvent::Key k) { return InputEvent{.type = type, .key = k}; }
  InputEvent text(std::string s) { return InputEvent{.type = InputEvent::Type::TextInput, .text = std::move(s)}; }

}  // namespace

TEST(Focus, APressOnAClaimingActorGivesItTheKeyboard) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));

  f.send(down(InputEvent::Button::Left, Vec2{10, 20}));
  ASSERT_TRUE(actor.focused_);
  EXPECT_EQ(actor.lastFocusPos_, (Vec2{10, 20})) << "the press arrives in the actor's parent space";

  EXPECT_TRUE(f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::Return)));
  EXPECT_TRUE(f.send(text("hi")));
  EXPECT_EQ(actor.keys_, std::vector<InputEvent::Key>{InputEvent::Key::Return});
  EXPECT_EQ(actor.typed_, "hi");
}

TEST(Focus, KeysAreUnconsumedUntilSomethingIsFocused) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));

  EXPECT_FALSE(f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::F)));
  EXPECT_FALSE(f.send(text("x")));
  EXPECT_TRUE(actor.keys_.empty());
  EXPECT_EQ(actor.typed_, "");
}

TEST(Focus, APressOnANonClaimingActorLeavesNoFocus) {
  Fixture f;
  f.root.add(std::make_unique<PlainActor>(Rect{0, 0, 50, 50}));

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));

  EXPECT_FALSE(f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::F)));
}

TEST(Focus, APressElsewhereBlursTheFocusedActor) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));
  f.root.add(std::make_unique<PlainActor>(Rect{100, 0, 50, 50}));

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  ASSERT_TRUE(actor.focused_);

  f.send(down(InputEvent::Button::Left, Vec2{110, 10}));

  EXPECT_FALSE(actor.focused_);
  EXPECT_FALSE(f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::F)));
}

TEST(Focus, KeysReachOnlyTheFocusedActor) {
  Fixture f;
  auto& first = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));
  auto& second = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{100, 0, 50, 50})));

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  f.send(text("a"));
  f.send(down(InputEvent::Button::Left, Vec2{110, 10}));
  f.send(text("b"));

  EXPECT_EQ(first.typed_, "a");
  EXPECT_EQ(second.typed_, "b");
  EXPECT_FALSE(first.focused_);
  EXPECT_TRUE(second.focused_);
}

TEST(Focus, RePressingTheFocusedActorReOffersFocusWithoutBlurring) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  f.send(down(InputEvent::Button::Left, Vec2{30, 10}));

  EXPECT_TRUE(actor.focused_);
  EXPECT_EQ(actor.lastFocusPos_, (Vec2{30, 10})) << "the second press re-aims the same focus";
}

// A re-press the focused actor declines (its grip, say) gives the focus up.
TEST(Focus, ADeclinedReOfferBlursTheFocusedActor) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  ASSERT_TRUE(actor.focused_);

  actor.claimsFocus_ = false;
  f.send(down(InputEvent::Button::Left, Vec2{30, 10}));

  EXPECT_FALSE(actor.focused_);
  EXPECT_FALSE(f.send(text("x")));
}

TEST(Focus, ResetBlursTheFocusedActor) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  ASSERT_TRUE(actor.focused_);

  f.chain.reset();

  EXPECT_FALSE(actor.focused_);
  EXPECT_FALSE(f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::F)));
}

TEST(Focus, DropFocusBlursWithoutTouchingGestureState) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));
  actor.claimsDrag_ = true;

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  ASSERT_TRUE(actor.focused_);
  ASSERT_TRUE(actor.dragging_);

  f.chain.dropFocus();

  EXPECT_FALSE(actor.focused_);
  EXPECT_TRUE(actor.dragging_) << "dropping focus is not a gesture cancel";
}

// Focus is additive: the press it claims still feeds drag and click.
TEST(Focus, ClaimingFocusDoesNotConsumeThePress) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));
  actor.claimsDrag_ = true;

  ASSERT_TRUE(f.send(down(InputEvent::Button::Left, Vec2{10, 10})));
  EXPECT_TRUE(actor.dragging_) << "the drag still claimed the same press";

  f.send(up(InputEvent::Button::Left, Vec2{10, 10}));
  EXPECT_TRUE(actor.focused_) << "ending the gesture does not end the edit";
}

// Space is an ordinary key: a focused field eats it, nothing else wants it.
TEST(Focus, SpaceIsJustAKeyForTheFocusedActor) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));

  EXPECT_FALSE(f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::Space)));

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  EXPECT_TRUE(f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::Space)));

  f.send(key(InputEvent::Type::KeyUp, InputEvent::Key::Space));
  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  EXPECT_TRUE(actor.focused_) << "the press is an edit: nothing pans on Space";
  EXPECT_EQ(f.view.pan, (Vec2{0, 0}));
}

// The actor goes away under the focus; the keyboard must come back unowned.
TEST(Focus, DetachingTheFocusedActorDropsTheFocus) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  ASSERT_TRUE(actor.focused_);

  std::unique_ptr<Actor> detached = f.root.detach(actor);
  ASSERT_NE(detached, nullptr);

  EXPECT_FALSE(f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::F)));
  EXPECT_FALSE(actor.focused_);

  // Re-attaching must not revive the focus the detach dropped.
  f.root.insert(0, std::move(detached));
  EXPECT_FALSE(f.send(text("x")));
  EXPECT_EQ(actor.typed_, "");
}
