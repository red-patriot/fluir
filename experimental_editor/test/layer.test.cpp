#include "editor/core/layer.hpp"

#include <vector>

#include <gtest/gtest.h>

#include "editor/actors/actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"
#include "recording_renderer.hpp"

namespace {

  using fluir::editor::Actor;
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
  // world/local pos it was clicked at.
  class StubActor : public Actor {
   public:
    explicit StubActor(Rect bounds) : Actor(bounds) { }

    void onClick(Vec2 pos) override {
      clicked_ = true;
      lastClickPos_ = pos;
    }
    void draw(const Subview& body, const EditorContext&) const override {
      body.renderer().fillRect(body.toScreen(bounds()), {});
    }

    bool clicked_ = false;
    Vec2 lastClickPos_{};
  };

}  // namespace

TEST(Layer, DrawCallsEachActorInListOrder) {
  StubActor a{Rect{0, 0, 10, 10}};
  StubActor b{Rect{20, 0, 10, 10}};
  RecordingRenderer renderer;
  EditorContext ctx;

  Layer layer;
  layer.setActors({&a, &b});
  layer.draw(renderer, ctx, Rect{0, 0, 100, 100});

  EXPECT_TRUE(hasFill(renderer.calls, Rect{0, 0, 10, 10}));
  EXPECT_TRUE(hasFill(renderer.calls, Rect{20, 0, 10, 10}));
}

TEST(Layer, TopmostAtReturnsLastPaintedActorOnOverlap) {
  StubActor bottom{Rect{0, 0, 10, 10}};
  StubActor top{Rect{0, 0, 10, 10}};

  Layer layer;
  layer.setActors({&bottom, &top});

  EXPECT_EQ(layer.topmostAt(Vec2{5, 5}), &top);
}

TEST(Layer, TopmostAtReturnsNullptrOnMiss) {
  StubActor a{Rect{0, 0, 10, 10}};

  Layer layer;
  layer.setActors({&a});

  EXPECT_EQ(layer.topmostAt(Vec2{500, 500}), nullptr);
}

TEST(Layer, TopmostAtConvertsScreenPosThroughViewport) {
  // pan={100,50}, scale=2: screenToWorld({120,70}) == {10,10}, inside the actor.
  StubActor a{Rect{0, 0, 20, 20}};

  Layer layer;
  layer.setViewport(Viewport{.pan = {100, 50}, .scale = 2.0});
  layer.setActors({&a});

  EXPECT_EQ(layer.topmostAt(Vec2{120, 70}), &a);
  EXPECT_EQ(layer.topmostAt(Vec2{5, 5}), nullptr);  // would hit at scale 1, not at scale 2
}

TEST(Layer, OnClickInvokedSeparatelyFromTopmostAt) {
  // Layer::topmostAt only finds the actor; dispatch is the caller's job (matches
  // GraphScene::topmostAt's existing contract).
  StubActor a{Rect{0, 0, 10, 10}};

  Layer layer;
  layer.setActors({&a});

  Actor* hit = layer.topmostAt(Vec2{5, 5});
  ASSERT_NE(hit, nullptr);
  hit->onClick(Vec2{5, 5});

  EXPECT_TRUE(a.clicked_);
}

TEST(Layer, HandleEventDispatchesLeftClickToTopmostActor) {
  StubActor a{Rect{0, 0, 10, 10}};

  Layer layer;
  layer.setActors({&a});

  const InputEvent event{.type = InputEvent::Type::MouseDown, .button = InputEvent::Button::Left, .pos = {5, 5}};

  EXPECT_TRUE(layer.handleEvent(event));
  EXPECT_TRUE(a.clicked_);
}

TEST(Layer, HandleEventReturnsFalseOnMiss) {
  StubActor a{Rect{0, 0, 10, 10}};

  Layer layer;
  layer.setActors({&a});

  const InputEvent event{.type = InputEvent::Type::MouseDown, .button = InputEvent::Button::Left, .pos = {500, 500}};

  EXPECT_FALSE(layer.handleEvent(event));
  EXPECT_FALSE(a.clicked_);
}

TEST(Layer, HandleEventIgnoresNonLeftClick) {
  StubActor a{Rect{0, 0, 10, 10}};

  Layer layer;
  layer.setActors({&a});

  const InputEvent move{.type = InputEvent::Type::MouseMove, .pos = {5, 5}};
  const InputEvent rightDown{.type = InputEvent::Type::MouseDown, .button = InputEvent::Button::Right, .pos = {5, 5}};

  EXPECT_FALSE(layer.handleEvent(move));
  EXPECT_FALSE(layer.handleEvent(rightDown));
  EXPECT_FALSE(a.clicked_);
}

TEST(Layer, HandleEventDispatchesInViewportWorldCoords) {
  // pan={100,50}, scale=2: screenToWorld({120,70}) == {10,10}.
  StubActor a{Rect{0, 0, 20, 20}};

  Layer layer;
  layer.setViewport(Viewport{.pan = {100, 50}, .scale = 2.0});
  layer.setActors({&a});

  const InputEvent event{.type = InputEvent::Type::MouseDown, .button = InputEvent::Button::Left, .pos = {120, 70}};

  EXPECT_TRUE(layer.handleEvent(event));
  EXPECT_EQ(a.lastClickPos_, (Vec2{10, 10}));
}
