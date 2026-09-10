#include "editor/pages/module.hpp"

#include <algorithm>
#include <filesystem>
#include <string>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/utility/context.hpp"
#include "editor/actors/actor.hpp"
#include "editor/actors/node_actors.hpp"
#include "editor/actors/scene.hpp"
#include "editor/core/collecting_sink.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/loader.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"
#include "fixture_loader.hpp"
#include "recording_renderer.hpp"

// These tests assert *ModulePage's MouseDown dispatch*: a plain Left click on
// an actor is consumed (not treated as a pan gesture), an existing pan
// gesture (Middle, or Left+Space) still pans even when it starts over a node,
// a Left click outside every actor stays inert, and Quit/Escape still forces
// EditorContext::running false regardless of where the cursor sits. All
// black-box, via RecordingRenderer's recorded frames and EditorContext state
// -- ModulePage exposes no scene/actor accessors.

namespace {

  using testutil::Loaded;
  using testutil::loadFixture;

  namespace fs = std::filesystem;

  using fluir::editor::Actor;
  using fluir::editor::ConstantActor;
  using fluir::editor::EditorContext;
  using fluir::editor::GraphScene;
  using fluir::editor::InputEvent;
  using fluir::editor::ModulePage;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::RecordingRenderer;

  // Replicates the exact fit-to-window transform ModulePage::start() applies
  // internally (fitRect(scene.worldBounds(), renderer.outputSize())), so a
  // test can turn a known world-space point (e.g. a verified actor rect, same
  // values asserted in scene.test.cpp / graph_geometry.test.cpp) into the
  // screen-space InputEvent position that will land on it.
  Vec2 toScreen(const EditorContext& ctx, const fluir::pt::ParseTree& tree, Vec2 outputSize, Vec2 world) {
    GraphScene scene;
    scene.build(ctx, tree);
    Viewport v;
    v.fitRect(scene.worldBounds(), outputSize);
    return v.worldToScreen(world);
  }

  const fs::path kIntConstants = fs::path(TEST_FOLDER) / "read/int_constants.fl";

  // int_constants.fl constant id=1 absolute rect {60,175,25,25} (verified in
  // scene.test.cpp / graph_draw.test.cpp / graph_geometry.test.cpp). The
  // drag handle is the 15px grip inset one unit from the node's top and right
  // edges -> world {65,180,15,15}; this point sits inside the node body but
  // OUTSIDE that handle, so a Left press here is a plain body click, not a
  // handle grab.
  constexpr Vec2 kInsideActor{80, 195};
  constexpr Vec2 kOutsideEveryActor{-1000, -1000};

  InputEvent mouseDown(InputEvent::Button button, Vec2 pos) {
    InputEvent ie;
    ie.type = InputEvent::Type::MouseDown;
    ie.button = button;
    ie.pos = pos;
    return ie;
  }

  InputEvent mouseUp(InputEvent::Button button, Vec2 pos) {
    InputEvent ie;
    ie.type = InputEvent::Type::MouseUp;
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

  page.draw();
  const auto before = renderer.calls;
  renderer.calls.clear();

  const Vec2 clickPos = toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideActor);
  page.update({mouseDown(InputEvent::Button::Left, clickPos), mouseMove(clickPos + Vec2{50, 50})});

  page.draw();
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

  page.draw();
  const auto before = renderer.calls;
  renderer.calls.clear();

  const Vec2 panStart = toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideActor);
  page.update({mouseDown(InputEvent::Button::Middle, panStart), mouseMove(panStart + Vec2{50, 50})});

  page.draw();
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

  page.draw();
  const auto before = renderer.calls;
  renderer.calls.clear();

  const Vec2 outsidePos = toScreen(ctx, *l.result.tree, renderer.outputSize_, kOutsideEveryActor);
  page.update({mouseDown(InputEvent::Button::Left, outsidePos), mouseMove(outsidePos + Vec2{50, 50})});

  page.draw();
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
  // {80,195} is inside it (same fixture/geometry verified in scene.test.cpp).
  Actor* hit = page.scene().topmostAt(kInsideActor);
  ASSERT_NE(hit, nullptr);
  auto* constant = dynamic_cast<ConstantActor*>(hit);
  ASSERT_NE(constant, nullptr);
  // id=1 is declared as <i8>-5</i8>.
  EXPECT_EQ(constant->lastClickSummary(), "constant -5");
}

TEST(ModulePage, LeftDragOnHandleMovesNode) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  // constant id=1 world rect {60,175,25,25}; drag handle is the 3x3-unit grip
  // inset one unit from the node's top and right edges -> world {65,180,15,15}.
  Actor* c = page.scene().topmostAt(Vec2{80, 195});
  ASSERT_NE(c, nullptr);
  const fluir::editor::Rect r0 = c->worldBounds();

  const auto S = [&](Vec2 w) { return toScreen(ctx, *l.result.tree, renderer.outputSize_, w); };
  page.update({mouseDown(InputEvent::Button::Left, S({70, 185})),  // inside the handle
               mouseMove(S({86, 185})),                            // +16 world x -> +3 grid units
               mouseUp(InputEvent::Button::Left, S({86, 185}))});

  EXPECT_EQ(c->worldBounds(), (fluir::editor::Rect{r0.x + 15, r0.y, r0.w, r0.h}));
}

TEST(ModulePage, ChildStaysDraggableAfterFrameDrag) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);
  page.draw();

  const auto S = [&](Vec2 w) { return toScreen(ctx, *l.result.tree, renderer.outputSize_, w); };

  page.update({mouseDown(InputEvent::Button::Left, S({537, 62})),
               mouseMove(S({552, 62})),
               mouseUp(InputEvent::Button::Left, S({552, 62}))});
  page.draw();  // reconciles child bounds_ from the moved frame origin

  Actor* child = page.scene().topmostAt(Vec2{92, 187});
  ASSERT_NE(child, nullptr);
  ASSERT_NE(dynamic_cast<ConstantActor*>(child), nullptr);

  ASSERT_TRUE(child->onDragStart(ctx, child->toParentLocal(Vec2{87, 187})));
  const fluir::editor::Rect r0 = child->worldBounds();
  child->onDrag(ctx, Vec2{}, Vec2{ctx.layout.unitPx, 0});  // +1 grid unit
  page.scene().layout(ctx);
  EXPECT_EQ(child->worldBounds(), (fluir::editor::Rect{r0.x + ctx.layout.unitPx, r0.y, r0.w, r0.h}));
}

TEST(ModulePage, LeftClickOnExitButtonClosesPage) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  page.draw();  // Lays out header_/exitButton_ bounds via HeaderBar::draw.

  const Vec2 clickPos = page.header().exitButton().bounds().center();
  page.update({mouseDown(InputEvent::Button::Left, clickPos)});

  EXPECT_TRUE(page.next());
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

namespace {

  // Loads a .fl file from an arbitrary absolute path (loadFixture only reaches
  // under TEST_FOLDER), version checks off like every other loader in this file.
  fluir::editor::LoadResult reloadFrom(fluir::editor::CollectingSink& sink, const fs::path& path) {
    fluir::Context ctx{
      .diagnosticSink = sink,
      .symbolTable = {},
      .currentFile = {},
      .outputFilename = {},
      .version = {},
      .ignoreVersionChecks = true,
    };
    return fluir::editor::loadFile(ctx, path);
  }

}  // namespace

// Save As is dialog-driven (NFD_SaveDialogU8) so it is not unit-tested here;
// saveToPath()/syncTreeFromScene() are exercised transitively via the Save
// button, whose path comes from EditorContext::program.
TEST(ModulePage, SaveWritesCurrentProgramToItsPath) {
  const fs::path tmp = fs::temp_directory_path() / "fluir_module_save_test.fl";
  fs::copy_file(kIntConstants, tmp, fs::copy_options::overwrite_existing);

  EditorContext ctx;
  ctx.program = tmp;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);
  page.draw();  // lays out header_/saveButton_ bounds

  const Vec2 clickPos = page.header().saveButton().bounds().center();
  page.update({mouseDown(InputEvent::Button::Left, clickPos)});

  fluir::editor::CollectingSink sink;
  const auto reloaded = reloadFrom(sink, tmp);
  EXPECT_TRUE(reloaded.tree.has_value());
  EXPECT_FALSE(reloaded.tree->declarations.empty());

  fs::remove(tmp);
}

TEST(ModulePage, SaveAfterDragPersistsNewPosition) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const fs::path tmp = fs::temp_directory_path() / "fluir_module_save_drag_test.fl";
  fs::copy_file(kIntConstants, tmp, fs::copy_options::overwrite_existing);

  EditorContext ctx;
  ctx.program = tmp;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);
  page.draw();

  // constant id=1 body-local x=2 (grid units) in int_constants.fl; drag its
  // handle +16 world x -> +3 grid units (same geometry as LeftDragOnHandleMovesNode).
  const auto S = [&](Vec2 w) { return toScreen(ctx, *l.result.tree, renderer.outputSize_, w); };
  page.update({mouseDown(InputEvent::Button::Left, S({70, 185})),
               mouseMove(S({86, 185})),
               mouseUp(InputEvent::Button::Left, S({86, 185}))});

  page.update({mouseDown(InputEvent::Button::Left, page.header().saveButton().bounds().center())});

  fluir::editor::CollectingSink sink;
  const auto reloaded = reloadFrom(sink, tmp);
  ASSERT_TRUE(reloaded.tree.has_value());

  const auto& fn = std::get<fluir::pt::FunctionDecl>(reloaded.tree->declarations.at(1));
  const auto& node = std::get<fluir::pt::Constant>(fn.body.nodes.at(1));
  EXPECT_EQ(node.location.x, 5);  // original 2 + 3 grid-unit drag

  fs::remove(tmp);
}
