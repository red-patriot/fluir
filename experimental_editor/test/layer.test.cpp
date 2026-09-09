#include "editor/core/layer.hpp"

#include <memory>

#include <gtest/gtest.h>

#include "editor/actors/actor.hpp"
#include "editor/components/container_actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/interaction.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"
#include "recording_renderer.hpp"

namespace {

  using fluir::editor::Actor;
  using fluir::editor::ClickInteraction;
  using fluir::editor::ContainerActor;
  using fluir::editor::EditorContext;
  using fluir::editor::InputEvent;
  using fluir::editor::Layer;
  using fluir::editor::Rect;
  using fluir::editor::Subview;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::hasFill;
  using testutil::RecordingRenderer;

  // Minimal Actor double: fillRect's its bounds when drawn, records the
  // parent-local pos it was clicked at.
  class StubActor : public Actor {
   public:
    explicit StubActor(Rect bounds) : Actor(bounds) { }

    void onClick(Vec2 pos) override {
      clicked_ = true;
      lastClickPos_ = pos;
    }
    void drawSelf(const Subview& body, const EditorContext&) const override {
      body.renderer().fillRect(body.toScreen(bounds()), {});
    }

    bool clicked_ = false;
    Vec2 lastClickPos_{};
  };

  // A screen-spanning root that neither offsets nor clips, matching how pages
  // root their chrome layers.
  struct Fixture {
    ContainerActor root{Rect{0, 0, 0, 0}, Actor::ClipChildren::No};
    Layer layer;

    StubActor& add(Rect bounds) { return static_cast<StubActor&>(root.add(std::make_unique<StubActor>(bounds))); }
    void wire() {
      layer.setRoot(root);
      layer.add(std::make_unique<ClickInteraction>());
    }
  };

  InputEvent leftDown(Vec2 pos) {
    return InputEvent{.type = InputEvent::Type::MouseDown, .button = InputEvent::Button::Left, .pos = pos};
  }

}  // namespace

TEST(Layer, DrawCallsEachActorInListOrder) {
  Fixture f;
  f.add(Rect{0, 0, 10, 10});
  f.add(Rect{20, 0, 10, 10});
  f.wire();

  RecordingRenderer renderer;
  EditorContext ctx;
  f.layer.draw(renderer, ctx, Rect{0, 0, 100, 100});

  EXPECT_TRUE(hasFill(renderer.calls, Rect{0, 0, 10, 10}));
  EXPECT_TRUE(hasFill(renderer.calls, Rect{20, 0, 10, 10}));
}

TEST(Layer, TopmostAtReturnsLastPaintedActorOnOverlap) {
  Fixture f;
  f.add(Rect{0, 0, 10, 10});
  StubActor& top = f.add(Rect{0, 0, 10, 10});
  f.wire();

  EXPECT_EQ(f.layer.topmostAt(Vec2{5, 5}), &top);
}

TEST(Layer, TopmostAtReturnsNullptrOnMiss) {
  Fixture f;
  f.add(Rect{0, 0, 10, 10});
  f.wire();

  EXPECT_EQ(f.layer.topmostAt(Vec2{500, 500}), nullptr);
}

TEST(Layer, TopmostAtConvertsScreenPosThroughViewport) {
  // pan={100,50}, scale=2: screenToWorld({120,70}) == {10,10}, inside the actor.
  Fixture f;
  StubActor& a = f.add(Rect{0, 0, 20, 20});
  f.wire();
  f.layer.setViewport(Viewport{.pan = {100, 50}, .scale = 2.0});

  EXPECT_EQ(f.layer.topmostAt(Vec2{120, 70}), &a);
  EXPECT_EQ(f.layer.topmostAt(Vec2{5, 5}), nullptr);  // would hit at scale 1, not at scale 2
}

TEST(Layer, DispatchSendsLeftClickToTopmostActor) {
  Fixture f;
  StubActor& a = f.add(Rect{0, 0, 10, 10});
  f.wire();

  EditorContext ctx;
  EXPECT_TRUE(f.layer.dispatch(leftDown(Vec2{5, 5}), ctx, Vec2{100, 100}));
  EXPECT_TRUE(a.clicked_);
}

TEST(Layer, DispatchReturnsFalseOnMiss) {
  Fixture f;
  StubActor& a = f.add(Rect{0, 0, 10, 10});
  f.wire();

  EditorContext ctx;
  EXPECT_FALSE(f.layer.dispatch(leftDown(Vec2{500, 500}), ctx, Vec2{100, 100}));
  EXPECT_FALSE(a.clicked_);
}

TEST(Layer, DispatchIgnoresNonLeftClick) {
  Fixture f;
  StubActor& a = f.add(Rect{0, 0, 10, 10});
  f.wire();

  EditorContext ctx;
  const InputEvent move{.type = InputEvent::Type::MouseMove, .pos = {5, 5}};
  const InputEvent rightDown{.type = InputEvent::Type::MouseDown, .button = InputEvent::Button::Right, .pos = {5, 5}};

  EXPECT_FALSE(f.layer.dispatch(move, ctx, Vec2{100, 100}));
  EXPECT_FALSE(f.layer.dispatch(rightDown, ctx, Vec2{100, 100}));
  EXPECT_FALSE(a.clicked_);
}

TEST(Layer, DispatchDeliversPositionsInTheActorsOwnSpace) {
  // pan={100,50}, scale=2: screenToWorld({120,70}) == {10,10}; the actor's
  // parent is the unclipped root at the origin, so parent space == world.
  Fixture f;
  StubActor& a = f.add(Rect{0, 0, 20, 20});
  f.wire();
  f.layer.setViewport(Viewport{.pan = {100, 50}, .scale = 2.0});

  EditorContext ctx;
  EXPECT_TRUE(f.layer.dispatch(leftDown(Vec2{120, 70}), ctx, Vec2{100, 100}));
  EXPECT_EQ(a.lastClickPos_, (Vec2{10, 10}));
}

TEST(Layer, DispatchWithoutARootIsInert) {
  Layer layer;
  EditorContext ctx;

  EXPECT_FALSE(layer.dispatch(leftDown(Vec2{5, 5}), ctx, Vec2{100, 100}));
  EXPECT_EQ(layer.topmostAt(Vec2{5, 5}), nullptr);
}
