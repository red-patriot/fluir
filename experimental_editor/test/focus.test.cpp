#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "editor/actors/actor.hpp"
#include "editor/components/container_actor.hpp"
#include "editor/components/text_field.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/interaction.hpp"
#include "editor/core/viewport.hpp"
#include "editor/gesture/gesture_host.hpp"
#include "editor/gesture/inline_edit.hpp"
#include "editor/input.hpp"
#include "stub_gesture.hpp"

// These tests assert the *focus contract*: which actor a press hands the
// keyboard to, when it is taken away again, that claiming focus never costs the
// actor the selection/drag/click dispatch it would otherwise get, and the key
// policy FocusInteraction owns -- Return commits, Escape cancels, the rest go
// to the draft.

namespace {

  using fluir::editor::Actor;
  using fluir::editor::ClickInteraction;
  using fluir::editor::ContainerActor;
  using fluir::editor::DragInteraction;
  using fluir::editor::EditorContext;
  using fluir::editor::FocusInteraction;
  using fluir::editor::GestureHost;
  using fluir::editor::InlineEdit;
  using fluir::editor::InputEvent;
  using fluir::editor::InteractionChain;
  using fluir::editor::InteractionContext;
  using fluir::editor::PanZoomInteraction;
  using fluir::editor::Rect;
  using fluir::editor::TextField;
  using fluir::editor::Vec2;
  using fluir::editor::Vec2i;
  using fluir::editor::Viewport;

  // A stub gesture edits nothing, so its size is never clamped.
  constexpr fluir::editor::Limits<Vec2i> kNoLimits{.lower = Vec2i{0, 0}, .upper = Vec2i{1000000, 1000000}};

  // Opens a draft anywhere inside itself, and records what it commits.
  class FocusableActor : public Actor {
   public:
    explicit FocusableActor(Rect bounds) : Actor(bounds) { }

    InlineEdit* editor() override { return &edit_; }
    GestureHost* gestures() override { return claimsDrag_ ? &gestures_ : nullptr; }
    void onClick(Vec2) override { ++clicks_; }

    bool focused() const { return edit_.active(); }
    bool dragging() const { return gestures_.active(); }
    /** The open draft's text, or "" when nothing is open. */
    std::string draft() const { return edit_.active() ? edit_.field()->text() : std::string{}; }
    /** The open draft's caret, or 0 when nothing is open. */
    std::size_t caret() const { return edit_.active() ? edit_.field()->caret() : 0u; }

    bool claimsFocus_ = true;
    bool claimsDrag_ = false; /**< a drag captures the chain, so keys stop flowing */
    bool accepts_ = true;     /**< whether this actor's validator takes the draft */
    std::string prefill_;     /**< what a press opens the draft on */
    std::vector<std::string> committed_;
    int clicks_ = 0;

   private:
    InlineEdit edit_{[this] { return claimsFocus_ ? std::optional<std::string>{prefill_} : std::nullopt; },
                     [this](const EditorContext&, const std::string& text) {
                       committed_.push_back(text);
                       return accepts_;
                     }};
    testutil::GestureLog log_;
    // The grip is the actor's right half, so a press left of x = 25 is a body
    // press that opens the draft and one right of it is a gesture press.
    GestureHost gestures_{*this, {testutil::loggingGrip(log_, Rect{5, 0, 5, 10})}, kNoLimits};
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
  ASSERT_TRUE(actor.focused());

  EXPECT_TRUE(f.send(text("hi")));
  EXPECT_EQ(actor.draft(), "hi");

  EXPECT_TRUE(f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::Return)));
  EXPECT_EQ(actor.committed_, std::vector<std::string>{"hi"});
  EXPECT_FALSE(actor.focused()) << "an accepted commit closes the draft";
}

TEST(Focus, EscapeClosesTheDraftWithoutCommitting) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  f.send(text("hi"));

  EXPECT_TRUE(f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::Escape)));

  EXPECT_FALSE(actor.focused());
  EXPECT_TRUE(actor.committed_.empty());
}

// The validator is the actor's own; a rejection is not the policy's business
// beyond leaving the draft where the user can fix it.
TEST(Focus, ARejectedCommitKeepsTheDraftFocused) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));
  actor.accepts_ = false;

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  f.send(text("no"));

  EXPECT_TRUE(f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::Return)));

  EXPECT_TRUE(actor.focused()) << "a rejected draft stays open to be fixed";
  EXPECT_EQ(actor.draft(), "no");
  EXPECT_EQ(actor.committed_, std::vector<std::string>{"no"});
}

TEST(Focus, KeysAreUnconsumedUntilSomethingIsFocused) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));

  EXPECT_FALSE(f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::F)));
  EXPECT_FALSE(f.send(text("x")));
  EXPECT_FALSE(actor.focused());
}

// An actor may hold an editor and still have nothing to edit -- a float
// constant, say. It declines, and the keyboard stays unowned.
TEST(Focus, AnEditorWithNothingEditableClaimsNoFocus) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));
  actor.claimsFocus_ = false;

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));

  EXPECT_FALSE(actor.focused());
  EXPECT_FALSE(f.send(text("x")));
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
  ASSERT_TRUE(actor.focused());

  f.send(down(InputEvent::Button::Left, Vec2{110, 10}));

  EXPECT_FALSE(actor.focused());
  EXPECT_FALSE(f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::F)));
}

TEST(Focus, KeysReachOnlyTheFocusedActor) {
  Fixture f;
  auto& first = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));
  auto& second = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{100, 0, 50, 50})));

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  f.send(text("a"));
  EXPECT_EQ(first.draft(), "a");

  f.send(down(InputEvent::Button::Left, Vec2{110, 10}));
  f.send(text("b"));

  EXPECT_EQ(second.draft(), "b");
  EXPECT_FALSE(first.focused()) << "the press elsewhere dropped the first draft";
  EXPECT_TRUE(second.focused());
}

TEST(Focus, RePressingTheFocusedActorReOffersFocusWithoutBlurring) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));

  actor.prefill_ = "abcd";  // 8px glyphs, so the caret reports where the press landed

  f.send(down(InputEvent::Button::Left, Vec2{8, 10}));
  ASSERT_EQ(actor.caret(), 1u);

  f.send(down(InputEvent::Button::Left, Vec2{16, 10}));

  EXPECT_TRUE(actor.focused());
  EXPECT_EQ(actor.draft(), "abcd") << "the draft survives the second press";
  EXPECT_EQ(actor.caret(), 2u) << "the second press re-aims the same focus";
}

// A press on a grip is a gesture, never an edit -- even on the actor that
// already holds the keyboard, which gives the focus up.
TEST(Focus, APressOnAGripBlursTheFocusedActor) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));
  actor.claimsDrag_ = true;

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  ASSERT_TRUE(actor.focused()) << "the body press opened a draft";

  f.send(down(InputEvent::Button::Left, Vec2{30, 10}));

  EXPECT_FALSE(actor.focused());
  EXPECT_TRUE(actor.dragging()) << "the same press started the gesture instead";
  EXPECT_FALSE(f.send(text("x")));
}

TEST(Focus, APressOnAGripNeverOpensADraft) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));
  actor.claimsDrag_ = true;

  f.send(down(InputEvent::Button::Left, Vec2{30, 10}));

  EXPECT_FALSE(actor.focused());
  EXPECT_TRUE(actor.dragging());
}

TEST(Focus, ResetBlursTheFocusedActor) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  ASSERT_TRUE(actor.focused());

  f.chain.reset();

  EXPECT_FALSE(actor.focused());
  EXPECT_FALSE(f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::F)));
}

TEST(Focus, DropFocusLeavesALiveGestureAlone) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));
  actor.claimsDrag_ = true;

  f.send(down(InputEvent::Button::Left, Vec2{30, 10}));
  ASSERT_TRUE(actor.dragging());

  f.chain.dropFocus();

  EXPECT_TRUE(actor.dragging()) << "dropping focus is not a gesture cancel";
}

// Focus is additive: the press it claims still feeds selection and click.
TEST(Focus, ClaimingFocusDoesNotConsumeThePress) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));

  ASSERT_TRUE(f.send(down(InputEvent::Button::Left, Vec2{10, 10})));

  EXPECT_TRUE(actor.focused());
  EXPECT_EQ(actor.clicks_, 1) << "the click still saw the same press";
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
  EXPECT_TRUE(actor.focused()) << "the press is an edit: nothing pans on Space";
  EXPECT_EQ(f.view.pan, (Vec2{0, 0}));
}

// The actor goes away under the focus; the keyboard must come back unowned.
TEST(Focus, DetachingTheFocusedActorDropsTheFocus) {
  Fixture f;
  auto& actor = static_cast<FocusableActor&>(f.root.add(std::make_unique<FocusableActor>(Rect{0, 0, 50, 50})));

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  ASSERT_TRUE(actor.focused());

  std::unique_ptr<Actor> detached = f.root.detach(actor);
  ASSERT_NE(detached, nullptr);

  EXPECT_FALSE(f.send(key(InputEvent::Type::KeyDown, InputEvent::Key::F)));
  EXPECT_FALSE(actor.focused());

  // Re-attaching must not revive the focus the detach dropped.
  f.root.insert(0, std::move(detached));
  EXPECT_FALSE(f.send(text("x")));
  EXPECT_FALSE(actor.focused());
}
