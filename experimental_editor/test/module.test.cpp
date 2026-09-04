#include "editor/pages/module.hpp"

#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/utility/context.hpp"
#include "editor/actors/actor.hpp"
#include "editor/actors/node_actors.hpp"
#include "editor/core/collecting_sink.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/loader.hpp"
#include "editor/core/render_graph.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"
#include "recording_renderer.hpp"

// These tests assert *ModulePage's MouseDown dispatch*: a plain Left click on
// an actor is consumed (not treated as a pan gesture), an existing pan
// gesture (Middle, or Left+Space) still pans even when it starts over a node,
// a Left click outside every actor stays inert, and Quit/Escape still forces
// EditorContext::running false regardless of where the cursor sits. All
// black-box, via RecordingRenderer's recorded frames and EditorContext state
// -- ModulePage exposes no scene/actor accessors.

namespace {

  namespace fs = std::filesystem;

  using fluir::editor::Actor;
  using fluir::editor::ConstantActor;
  using fluir::editor::EditorContext;
  using fluir::editor::graphBounds;
  using fluir::editor::InputEvent;
  using fluir::editor::ModulePage;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::RecordingRenderer;

  // Same fixture-loading shape as scene.test.cpp / render_graph.test.cpp: each
  // load owns its Context + CollectingSink, version checks off (fixtures
  // declare <version>0.1.3</version>).
  struct Loaded {
    fluir::editor::CollectingSink sink;
    fluir::editor::LoadResult result;
  };

  Loaded loadFixture(const std::string& relPath) {
    Loaded l;
    fluir::Context ctx{
      .diagnosticSink = l.sink,
      .symbolTable = {},
      .currentFile = {},
      .outputFilename = {},
      .version = {},
      .ignoreVersionChecks = true,
    };
    l.result = fluir::editor::loadFile(ctx, fs::path(TEST_FOLDER) / relPath);
    return l;
  }

  // Replicates the exact fit-to-window transform ModulePage::start() applies
  // internally (fitRect(graphBounds(ctx, tree), renderer.outputSize())), so a
  // test can turn a known world-space point (e.g. a verified actor rect, same
  // values asserted in scene.test.cpp / graph_geometry.test.cpp) into the
  // screen-space InputEvent position that will land on it.
  Vec2 toScreen(const EditorContext& ctx, const fluir::pt::ParseTree& tree, Vec2 outputSize, Vec2 world) {
    Viewport v;
    v.fitRect(graphBounds(ctx, tree), outputSize);
    return v.worldToScreen(world);
  }

  const fs::path kIntConstants = fs::path(TEST_FOLDER) / "read/int_constants.fl";

  // int_constants.fl constant id=1 absolute rect {60,175,25,25} (verified in
  // scene.test.cpp / render_graph.test.cpp / graph_geometry.test.cpp).
  constexpr Vec2 kInsideActor{65, 180};
  constexpr Vec2 kOutsideEveryActor{-1000, -1000};

  InputEvent mouseDown(InputEvent::Button button, Vec2 pos) {
    InputEvent ie;
    ie.type = InputEvent::Type::MouseDown;
    ie.button = button;
    ie.pos = pos;
    return ie;
  }

  InputEvent mouseMove(Vec2 pos) {
    InputEvent ie;
    ie.type = InputEvent::Type::MouseMove;
    ie.pos = pos;
    return ie;
  }

  InputEvent quit() {
    InputEvent ie;
    ie.type = InputEvent::Type::Quit;
    return ie;
  }

}  // namespace

TEST(ModulePage, LeftClickOnActorDoesNotPan) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  page.write();
  const auto before = renderer.calls;
  renderer.calls.clear();

  const Vec2 clickPos = toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideActor);
  page.update({mouseDown(InputEvent::Button::Left, clickPos), mouseMove(clickPos + Vec2{50, 50})});

  page.write();
  const auto after = renderer.calls;

  EXPECT_EQ(before, after) << "plain Left click+drag on a node must not pan the view";
}

TEST(ModulePage, MiddlePanGestureOverActorStillPans) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  page.write();
  const auto before = renderer.calls;
  renderer.calls.clear();

  const Vec2 panStart = toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideActor);
  page.update({mouseDown(InputEvent::Button::Middle, panStart), mouseMove(panStart + Vec2{50, 50})});

  page.write();
  const auto after = renderer.calls;

  EXPECT_NE(before, after) << "a pan gesture starting over a node must still pan (regression guard)";
}

TEST(ModulePage, LeftClickOutsideEveryActorStaysInert) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  page.write();
  const auto before = renderer.calls;
  renderer.calls.clear();

  const Vec2 outsidePos = toScreen(ctx, *l.result.tree, renderer.outputSize_, kOutsideEveryActor);
  page.update({mouseDown(InputEvent::Button::Left, outsidePos), mouseMove(outsidePos + Vec2{50, 50})});

  page.write();
  const auto after = renderer.calls;

  EXPECT_EQ(before, after) << "Left click outside every actor must stay inert (no pan, no crash)";
  EXPECT_TRUE(ctx.running);
}

TEST(ModulePage, LeftClickOnActorDispatchesToItsOnClick) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  const Vec2 clickPos = toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideActor);
  page.update({mouseDown(InputEvent::Button::Left, clickPos)});

  // int_constants.fl constant id=1 absolute rect {60,175,25,25}; kInsideActor
  // {65,180} is inside it (same fixture/geometry verified in scene.test.cpp).
  Actor* hit = page.scene().topmostAt(kInsideActor);
  ASSERT_NE(hit, nullptr);
  auto* constant = dynamic_cast<ConstantActor*>(hit);
  ASSERT_NE(constant, nullptr);
  // id=1 is declared as <i8>-5</i8>.
  EXPECT_EQ(constant->lastClickSummary(), "constant -5");
}

TEST(ModulePage, QuitForcesRunningFalseEvenOverANode) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  const Vec2 overActor = toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideActor);
  page.update({mouseMove(overActor), quit()});

  EXPECT_FALSE(ctx.running);
}
