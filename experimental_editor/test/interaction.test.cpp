#include "editor/core/interaction.hpp"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
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
    void onClick(Vec2) override { ++clicks_; }

    bool dragging_ = false;
    Vec2 dragged_{};
    int clicks_ = 0;
  };

  // A draggable actor that also has an id and a position, as a node does.
  class MovableActor : public DraggableActor {
   public:
    MovableActor(Rect bounds, fluir::FullID id, int x, int y) :
      DraggableActor(bounds), id_(std::move(id)), location_{x, y, 0, 5, 5} { }

    bool onDragStart(const EditorContext& ctx, Vec2 pos) override {
      return claims_ && DraggableActor::onDragStart(ctx, pos);
    }
    void onDrag(const EditorContext& ctx, Vec2 pos, Vec2 delta) override {
      DraggableActor::onDrag(ctx, pos, delta);
      location_.x += static_cast<int>(delta.x);
      location_.y += static_cast<int>(delta.y);
    }
    fluir::FlowGraphLocation* location() override { return &location_; }
    std::optional<fluir::FullID> selectionId() const override { return id_; }

    bool claims_ = true;

   private:
    fluir::FullID id_;
    fluir::FlowGraphLocation location_;
  };

  // Claims a drag but names no position: not a move.
  class PositionlessActor : public DraggableActor {
   public:
    PositionlessActor(Rect bounds, fluir::FullID id) : DraggableActor(bounds), id_(std::move(id)) { }

    std::optional<fluir::FullID> selectionId() const override { return id_; }

   private:
    fluir::FullID id_;
  };

  // Records every move the drag interaction reports.
  struct MoveReport {
    fluir::FullID id;
    int startX = 0;
    int startY = 0;
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

    explicit Fixture(DragInteraction::MoveCommit onMoveCommit = {}) {
      chain.add(std::make_unique<PanZoomInteraction>());
      chain.add(std::make_unique<DragInteraction>(std::move(onMoveCommit)));
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

TEST(DragInteraction, ReportsThePreDragPositionOnRelease) {
  std::vector<MoveReport> reports;
  Fixture f{[&](const fluir::FullID& id, int startX, int startY) { reports.push_back({id, startX, startY}); }};
  auto& node = static_cast<MovableActor&>(
    f.root.add(std::make_unique<MovableActor>(Rect{0, 0, 50, 50}, fluir::FullID{1, 2}, 7, 9)));

  ASSERT_TRUE(f.send(down(InputEvent::Button::Left, Vec2{10, 10})));
  ASSERT_TRUE(f.send(move(Vec2{30, 40})));
  ASSERT_TRUE(f.send(up(InputEvent::Button::Left, Vec2{30, 40})));

  ASSERT_EQ(reports.size(), 1u);
  EXPECT_EQ(reports[0].id, (fluir::FullID{1, 2}));
  EXPECT_EQ(reports[0].startX, 7) << "the reported position is the one held at MouseDown";
  EXPECT_EQ(reports[0].startY, 9);
  EXPECT_EQ(node.location()->x, 27) << "the drag itself still wrote the final position";
}

TEST(DragInteraction, DoesNotReportAGestureTheActorRefused) {
  std::vector<MoveReport> reports;
  Fixture f{[&](const fluir::FullID& id, int startX, int startY) { reports.push_back({id, startX, startY}); }};
  auto& node = static_cast<MovableActor&>(
    f.root.add(std::make_unique<MovableActor>(Rect{0, 0, 50, 50}, fluir::FullID{1, 2}, 7, 9)));
  node.claims_ = false;

  f.send(down(InputEvent::Button::Left, Vec2{10, 10}));
  f.send(move(Vec2{30, 40}));
  f.send(up(InputEvent::Button::Left, Vec2{30, 40}));

  EXPECT_TRUE(reports.empty());
}

TEST(DragInteraction, DoesNotReportAnActorWithNoLocation) {
  std::vector<MoveReport> reports;
  Fixture f{[&](const fluir::FullID& id, int startX, int startY) { reports.push_back({id, startX, startY}); }};
  f.root.add(std::make_unique<PositionlessActor>(Rect{0, 0, 50, 50}, fluir::FullID{1, 2}));

  ASSERT_TRUE(f.send(down(InputEvent::Button::Left, Vec2{10, 10})));
  f.send(move(Vec2{30, 40}));
  ASSERT_TRUE(f.send(up(InputEvent::Button::Left, Vec2{30, 40})));

  EXPECT_TRUE(reports.empty());
}

TEST(DragInteraction, ResetCancelsThePendingReport) {
  std::vector<MoveReport> reports;
  Fixture f{[&](const fluir::FullID& id, int startX, int startY) { reports.push_back({id, startX, startY}); }};
  f.root.add(std::make_unique<MovableActor>(Rect{0, 0, 50, 50}, fluir::FullID{1, 2}, 7, 9));

  ASSERT_TRUE(f.send(down(InputEvent::Button::Left, Vec2{10, 10})));
  f.send(move(Vec2{30, 40}));
  f.chain.reset();
  f.send(up(InputEvent::Button::Left, Vec2{30, 40}));

  EXPECT_TRUE(reports.empty());
}
