#include <filesystem>
#include <memory>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/utility/context.hpp"
#include "editor/actors/actor.hpp"
#include "editor/actors/function_decl_actor.hpp"
#include "editor/actors/node_actor.hpp"
#include "editor/actors/scene.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/layer.hpp"
#include "editor/core/loader.hpp"
#include "fixture_loader.hpp"
#include "recording_renderer.hpp"

// Selection is stored on GraphScene as an id and cached as a flag on the Actor,
// so it survives the rebuild that every tree edit forces. The outline is
// geometric, not coloured: RecordingRenderer records no colour.

namespace {

  using testutil::Loaded;
  using testutil::loadFixture;

  using fluir::FlowGraphLocation;
  using fluir::ID;
  using fluir::editor::Actor;
  using fluir::editor::EditorContext;
  using fluir::editor::GraphScene;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using testutil::hasRect;
  using testutil::RecordingRenderer;

  const EditorContext kCtx;

  fluir::pt::Constant makeConstant(ID id, int x, int y, int z, int w, int h) {
    fluir::pt::Constant constant;
    constant.id = id;
    constant.location = FlowGraphLocation{.x = x, .y = y, .z = z, .width = w, .height = h};
    constant.value = fluir::literals_types::I32{0};
    return constant;
  }

  fluir::pt::FunctionDecl makeFunction(ID id, int width, int height, fluir::pt::Block body) {
    fluir::pt::FunctionDecl fn;
    fn.id = id;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = width, .height = height};
    fn.name = "f";
    fn.body = std::move(body);
    return fn;
  }

  fluir::pt::ParseTree singleFunctionTree(fluir::pt::FunctionDecl fn) {
    fluir::pt::ParseTree tree;
    tree.declarations.emplace(fn.id, fluir::pt::Declaration{fn});
    return tree;
  }

  // Two constants in one function, ids 10 and 11.
  fluir::pt::ParseTree twoNodeTree() {
    fluir::pt::Block body;
    body.nodes.emplace(10, makeConstant(10, 1, 1, 1, 2, 2));
    body.nodes.emplace(11, makeConstant(11, 10, 10, 1, 2, 2));
    return singleFunctionTree(makeFunction(1, 200, 200, std::move(body)));
  }

  // Draws `scene` through one Layer with an identity viewport, as ModulePage does.
  void drawScene(const GraphScene& scene, RecordingRenderer& r) {
    fluir::editor::Layer layer;
    layer.setRoot(scene.root());
    layer.draw(r, kCtx, Rect{0, 0, r.outputSize().x, r.outputSize().y});
  }

}  // namespace

TEST(SceneSelection, NothingIsSelectedAfterBuild) {
  GraphScene scene;
  scene.build(kCtx, twoNodeTree());

  EXPECT_FALSE(scene.selected().has_value());
  EXPECT_FALSE(scene.find(1, 10)->selected());
  EXPECT_FALSE(scene.find(1)->selected());
}

TEST(SceneSelection, SelectMarksTheNamedNodeActor) {
  GraphScene scene;
  scene.build(kCtx, twoNodeTree());

  scene.select(fluir::FullID{1, 10});

  ASSERT_TRUE(scene.selected().has_value());
  EXPECT_EQ(*scene.selected(), (fluir::FullID{1, 10}));
  EXPECT_TRUE(scene.find(1, 10)->selected());
  EXPECT_FALSE(scene.find(1, 11)->selected());
  EXPECT_FALSE(scene.find(1)->selected());
}

TEST(SceneSelection, SelectMarksAFunctionFrame) {
  GraphScene scene;
  scene.build(kCtx, twoNodeTree());

  scene.select(fluir::FullID{1});

  ASSERT_TRUE(scene.selected().has_value());
  EXPECT_EQ(*scene.selected(), (fluir::FullID{1}));
  EXPECT_TRUE(scene.find(1)->selected());
  EXPECT_FALSE(scene.find(1, 10)->selected());
}

TEST(SceneSelection, SelectingAnotherNodeDeselectsThePrevious) {
  GraphScene scene;
  scene.build(kCtx, twoNodeTree());

  scene.select(fluir::FullID{1, 10});
  scene.select(fluir::FullID{1, 11});

  EXPECT_FALSE(scene.find(1, 10)->selected());
  EXPECT_TRUE(scene.find(1, 11)->selected());
  EXPECT_EQ(*scene.selected(), (fluir::FullID{1, 11}));
}

TEST(SceneSelection, ClearSelectionUnmarksTheActor) {
  GraphScene scene;
  scene.build(kCtx, twoNodeTree());

  scene.select(fluir::FullID{1, 10});
  scene.clearSelection();

  EXPECT_FALSE(scene.selected().has_value());
  EXPECT_FALSE(scene.find(1, 10)->selected());
}

TEST(SceneSelection, SelectIgnoresAnUnknownId) {
  GraphScene scene;
  scene.build(kCtx, twoNodeTree());

  scene.select(fluir::FullID{1, 999});

  EXPECT_FALSE(scene.selected().has_value());
  EXPECT_FALSE(scene.find(1, 10)->selected());

  scene.select(fluir::FullID{1, 10});
  scene.select(fluir::FullID{999});

  // A failed select leaves the previous selection alone rather than half-clearing it.
  EXPECT_EQ(*scene.selected(), (fluir::FullID{1, 10}));
  EXPECT_TRUE(scene.find(1, 10)->selected());
}

TEST(SceneSelection, SelectionSurvivesARebuild) {
  GraphScene scene;
  scene.build(kCtx, twoNodeTree());
  scene.select(fluir::FullID{1, 10});

  scene.build(kCtx, twoNodeTree());

  ASSERT_TRUE(scene.selected().has_value());
  EXPECT_EQ(*scene.selected(), (fluir::FullID{1, 10}));
  // The new actor, never the stale pointer captured before the rebuild.
  EXPECT_TRUE(scene.find(1, 10)->selected());
}

// int_constants.fl: function main {50,50,500,500}; constant id=1 {60,175,25,25}.
// selectionPad is 2, so the outline rects sit 2 and 3 px outside those.

TEST(SelectionOutline, SelectedNodeDrawsTwoOutsetRects) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);
  scene.select(fluir::FullID{1, 1});

  RecordingRenderer r;
  drawScene(scene, r);

  EXPECT_TRUE(hasRect(r.calls, Rect{58, 173, 29, 29}));
  EXPECT_TRUE(hasRect(r.calls, Rect{57, 172, 31, 31}));
}

TEST(SelectionOutline, UnselectedNodeDrawsNoOutsetRect) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);

  RecordingRenderer r;
  drawScene(scene, r);

  EXPECT_FALSE(hasRect(r.calls, Rect{58, 173, 29, 29}));
  EXPECT_FALSE(hasRect(r.calls, Rect{57, 172, 31, 31}));
  EXPECT_TRUE(hasRect(r.calls, Rect{60, 175, 25, 25}));  // the node's own border still drawn
}

TEST(SelectionOutline, SelectedFrameDrawsTwoOutsetRects) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);
  scene.select(fluir::FullID{1});

  RecordingRenderer r;
  drawScene(scene, r);

  EXPECT_TRUE(hasRect(r.calls, Rect{48, 48, 504, 504}));
  EXPECT_TRUE(hasRect(r.calls, Rect{47, 47, 506, 506}));
}

TEST(SelectionOutline, DeselectingRemovesTheOutline) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  GraphScene scene;
  scene.build(kCtx, *l.result.tree);
  scene.select(fluir::FullID{1, 1});
  scene.clearSelection();

  RecordingRenderer r;
  drawScene(scene, r);

  EXPECT_FALSE(hasRect(r.calls, Rect{58, 173, 29, 29}));
  EXPECT_FALSE(hasRect(r.calls, Rect{57, 172, 31, 31}));
}
