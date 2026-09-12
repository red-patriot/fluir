#include "editor/pages/module.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/literal_types.hpp"
#include "compiler/utility/context.hpp"
#include "editor/actors/actor.hpp"
#include "editor/actors/node_actors.hpp"
#include "editor/actors/scene.hpp"
#include "editor/components/text_field.hpp"
#include "editor/core/collecting_sink.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/loader.hpp"
#include "editor/core/viewport.hpp"
#include "editor/gesture/inline_edit.hpp"
#include "editor/input.hpp"
#include "editor/transaction/delete.hpp"
#include "fixture_loader.hpp"
#include "recording_renderer.hpp"

// These tests assert *ModulePage's MouseDown dispatch*: a plain Left click on
// an actor is consumed (not treated as a pan gesture), an existing pan
// gesture (Middle) still pans even when it starts over a node,
// a Left click outside every actor stays inert, and Quit still forces
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
  using fluir::editor::InlineEdit;
  using fluir::editor::InputEvent;
  using fluir::editor::ModulePage;
  using fluir::editor::NodeActor;
  using fluir::editor::Rect;
  using fluir::editor::TextField;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::countOf;
  using testutil::DrawCall;
  using testutil::hasRect;
  using testutil::RecordingRenderer;
  using testutil::textStrings;

  // Replicates the exact fit-to-window transform ModulePage::start() applies
  // internally (fitRect(scene.worldBounds(), renderer.outputSize())), so a
  // test can turn a known world-space point (e.g. a verified actor rect, same
  // values asserted in scene.test.cpp / graph_geometry.test.cpp) into the
  // screen-space InputEvent position that will land on it.
  Viewport fitViewport(const EditorContext& ctx, const fluir::pt::ParseTree& tree, Vec2 outputSize) {
    GraphScene scene;
    scene.build(ctx, tree);
    Viewport v;
    v.fitRect(scene.worldBounds(), outputSize);
    return v;
  }

  Vec2 toScreen(const EditorContext& ctx, const fluir::pt::ParseTree& tree, Vec2 outputSize, Vec2 world) {
    return fitViewport(ctx, tree, outputSize).worldToScreen(world);
  }

  // `world`'s rect as the renderer sees it under that same fit transform.
  Rect toScreenRect(const Viewport& v, Rect world) {
    const Vec2 tl = v.worldToScreen(world.topLeft());
    return {tl.x, tl.y, world.w * v.scale, world.h * v.scale};
  }

  const fs::path kIntConstants = fs::path(TEST_FOLDER) / "read/int_constants.fl";
  const fs::path kSimpleBinary = fs::path(TEST_FOLDER) / "read/simple_binary_expr.fl";

  // simple_binary_expr.fl world rects (unitPx 5, body origin {50,75}):
  //   constant id=2 {60,85,25,25}, constant id=3 {60,135,25,25}, binary id=1 {125,85,25,25}.
  // Each point sits in the node's lower-left, clear of its top-right 15px grip.
  constexpr Vec2 kInsideConstant2{63, 103};
  constexpr Vec2 kInsideBinary1{128, 103};

  // int_constants.fl constant id=1 absolute rect {60,175,25,25} (verified in
  // scene.test.cpp / graph_draw.test.cpp / graph_geometry.test.cpp). The
  // drag handle is the 15px grip inset one unit from the node's top and right
  // edges -> world {65,180,15,15}, and the resize bar is the node's last 5px
  // column -> world {80,175,5,25}; this point sits inside the node body but
  // OUTSIDE both grips, so a Left press here is a plain body click.
  constexpr Vec2 kInsideActor{72, 197};
  constexpr Vec2 kOutsideEveryActor{-1000, -1000};
  // The node's own world rect, and the centre of its 15px drag grip.
  constexpr Rect kActorWorldRect{60, 175, 25, 25};
  constexpr Vec2 kOnDragHandle{72.5, 187.5};
  // Inside function main's frame {50,50,500,500}, far from every node.
  constexpr Vec2 kInsideEmptyFrameArea{400, 400};

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

  InputEvent keyDown(InputEvent::Key key) {
    InputEvent ie;
    ie.type = InputEvent::Type::KeyDown;
    ie.key = key;
    return ie;
  }

  InputEvent deleteKey() { return keyDown(InputEvent::Key::Delete); }

  InputEvent textInput(std::string text) {
    InputEvent ie;
    ie.type = InputEvent::Type::TextInput;
    ie.text = std::move(text);
    return ie;
  }

  InputEvent wheel(Vec2 pos, Vec2 delta) {
    InputEvent ie;
    ie.type = InputEvent::Type::Wheel;
    ie.pos = pos;
    ie.wheel = delta;
    return ie;
  }

  InputEvent quit() {
    InputEvent ie;
    ie.type = InputEvent::Type::Quit;
    return ie;
  }

  // int_constants.fl constant id=2 absolute rect {110,180,25,25}; this point is
  // in its lower-left, clear of the drag grip {115,185,15,15} and resize bar.
  constexpr Vec2 kInsideIntConstant2{113, 198};

  /** The in-place editor of function 1's constant `node`, or nullptr. */
  const InlineEdit* fieldOf(const ModulePage& page, fluir::ID node) {
    auto* actor = dynamic_cast<ConstantActor*>(page.scene().find(1, node));
    return actor == nullptr ? nullptr : actor->editor();
  }

  /** The live literal of function 1's constant `node`, or nullptr. */
  const fluir::pt::Literal* literalOf(const ModulePage& page, fluir::ID node) {
    auto* actor = dynamic_cast<ConstantActor*>(page.scene().find(1, node));
    return actor == nullptr ? nullptr : actor->literal();
  }

  /** `node`'s literal as an i8, which every int_constants.fl id=1 value is. */
  int i8Of(const ModulePage& page, fluir::ID node) {
    const fluir::pt::Literal* literal = literalOf(page, node);
    EXPECT_NE(literal, nullptr);
    return literal == nullptr ? 0 : std::get<fluir::literals_types::I8>(*literal);
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

  // A click legitimately adds a selection outline, so the contract is the
  // clicked node's *screen* rect -- which any pan would move.
  const Viewport v = fitViewport(ctx, *l.result.tree, renderer.outputSize_);
  const Rect nodeScreen = toScreenRect(v, kActorWorldRect);

  page.draw();
  ASSERT_TRUE(hasRect(renderer.calls, nodeScreen));
  renderer.calls.clear();

  const Vec2 clickPos = v.worldToScreen(kInsideActor);
  page.update({mouseDown(InputEvent::Button::Left, clickPos), mouseMove(clickPos + Vec2{50, 50})});

  page.draw();

  EXPECT_TRUE(hasRect(renderer.calls, nodeScreen)) << "plain Left click+drag on a node must not pan the view";
}

TEST(ModulePage, LeftClickOnNodeSelectsThatNode) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  page.update({mouseDown(InputEvent::Button::Left, toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideActor))});

  ASSERT_TRUE(page.scene().selected().has_value());
  EXPECT_EQ(*page.scene().selected(), (fluir::FullID{1, 1}));
}

TEST(ModulePage, LeftClickOnNodeSelectsTheNodeNotTheEnclosingFrame) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  page.update({mouseDown(InputEvent::Button::Left, toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideActor))});

  ASSERT_TRUE(page.scene().selected().has_value());
  EXPECT_NE(*page.scene().selected(), (fluir::FullID{1}));
  EXPECT_FALSE(page.scene().find(1)->selected());
  EXPECT_TRUE(page.scene().find(1, 1)->selected());
}

TEST(ModulePage, LeftClickOnEmptyFrameAreaSelectsTheFrame) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  page.update(
    {mouseDown(InputEvent::Button::Left, toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideEmptyFrameArea))});

  ASSERT_TRUE(page.scene().selected().has_value());
  EXPECT_EQ(*page.scene().selected(), (fluir::FullID{1}));
  EXPECT_TRUE(page.scene().find(1)->selected());
}

TEST(ModulePage, LeftClickOnBackgroundClearsTheSelection) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  page.update({mouseDown(InputEvent::Button::Left, toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideActor))});
  ASSERT_TRUE(page.scene().selected().has_value());

  page.update({mouseDown(InputEvent::Button::Left, kOutsideEveryActor)});

  EXPECT_FALSE(page.scene().selected().has_value());
  EXPECT_FALSE(page.scene().find(1, 1)->selected());
}

TEST(ModulePage, PressOnADragHandleAlsoSelectsTheNode) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  // SelectionInteraction must not consume the press, or the grip stops dragging.
  const Viewport v = fitViewport(ctx, *l.result.tree, renderer.outputSize_);
  const Vec2 grip = v.worldToScreen(kOnDragHandle);
  page.update({mouseDown(InputEvent::Button::Left, grip),
               mouseMove(grip + Vec2{50 * v.scale, 0}),
               mouseUp(InputEvent::Button::Left, grip + Vec2{50 * v.scale, 0})});

  ASSERT_TRUE(page.scene().selected().has_value());
  EXPECT_EQ(*page.scene().selected(), (fluir::FullID{1, 1}));
  EXPECT_EQ(page.scene().find(1, 1)->worldBounds().x, kActorWorldRect.x + 50);
}

// Space arms nothing: a held Space leaves a Left press an ordinary press.
TEST(ModulePage, SpaceLeftOverANodeStillSelectsIt) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  const Vec2 start = toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideActor);
  page.update({keyDown(InputEvent::Key::Space), mouseDown(InputEvent::Button::Left, start)});

  ASSERT_TRUE(page.scene().selected().has_value());
  EXPECT_EQ(*page.scene().selected(), (fluir::FullID{1, 1}));
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

  ASSERT_TRUE(child->gestures()->press(ctx, child->toParentLocal(Vec2{87, 187})));
  const fluir::editor::Rect r0 = child->worldBounds();
  child->gestures()->drag(ctx, Vec2{ctx.layout.unitPx, 0});  // +1 grid unit
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

// DELETE edits the in-memory tree and rebuilds the scene; it never touches the
// viewport, and reaches disk only on the next Save.

TEST(ModulePage, DeleteWithNoSelectionChangesNothing) {
  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  const fluir::editor::Rect boundsBefore = page.scene().worldBounds();
  page.update({deleteKey()});

  EXPECT_NE(page.scene().find(1, 1), nullptr);
  EXPECT_NE(page.scene().find(1), nullptr);
  EXPECT_EQ(page.scene().worldBounds(), boundsBefore);
}

TEST(ModulePage, DeleteRemovesTheSelectedNodeFromTheScene) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  // The body click opens this i8 constant's in-place editor, which owns the
  // keyboard; Escape closes it so Delete is a page command again.
  page.update({mouseDown(InputEvent::Button::Left, toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideActor)),
               keyDown(InputEvent::Key::Escape),
               deleteKey()});

  EXPECT_EQ(page.scene().find(1, 1), nullptr);
  EXPECT_NE(page.scene().find(1, 2), nullptr);  // its siblings survive
  EXPECT_NE(page.scene().find(1), nullptr);
}

TEST(ModulePage, DeleteWithNothingFocusedDeletesTheSelection) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  // A press on the drag grip selects without opening an editor, so Delete is
  // still the page's command.
  const Viewport v = fitViewport(ctx, *l.result.tree, renderer.outputSize_);
  const Vec2 grip = v.worldToScreen(kOnDragHandle);
  page.update({mouseDown(InputEvent::Button::Left, grip), mouseUp(InputEvent::Button::Left, grip), deleteKey()});

  EXPECT_EQ(page.scene().find(1, 1), nullptr);
}

TEST(ModulePage, DeleteClearsTheSelection) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  // The body click opens this i8 constant's in-place editor, which owns the
  // keyboard; Escape closes it so Delete is a page command again.
  page.update({mouseDown(InputEvent::Button::Left, toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideActor)),
               keyDown(InputEvent::Key::Escape),
               deleteKey()});

  EXPECT_FALSE(page.scene().selected().has_value());
}

TEST(ModulePage, DeleteRemovesConduitsAttachedToTheDeletedNode) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kSimpleBinary;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  page.draw();
  ASSERT_EQ(countOf(renderer.calls, DrawCall::Op::Line), 2u);
  renderer.calls.clear();

  // Both conduits target the binary node, so both go with it.
  page.update({mouseDown(InputEvent::Button::Left, toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideBinary1)),
               deleteKey()});
  page.draw();

  EXPECT_EQ(countOf(renderer.calls, DrawCall::Op::Line), 0u);
}

TEST(ModulePage, DeleteOfAConstantRemovesOnlyItsOwnConduit) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kSimpleBinary;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  page.update(
    {mouseDown(InputEvent::Button::Left, toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideConstant2)),
     deleteKey()});
  page.draw();

  EXPECT_EQ(countOf(renderer.calls, DrawCall::Op::Line), 1u) << "the sibling constant's wire survives";
  EXPECT_EQ(page.scene().find(1, 2), nullptr);
  EXPECT_NE(page.scene().find(1, 3), nullptr);
}

TEST(ModulePage, DeleteOfAFrameRemovesTheWholeFunction) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  page.update(
    {mouseDown(InputEvent::Button::Left, toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideEmptyFrameArea)),
     deleteKey()});
  renderer.calls.clear();
  page.draw();

  EXPECT_EQ(page.scene().find(1), nullptr);
  EXPECT_EQ(page.scene().find(1, 1), nullptr);
  EXPECT_EQ(page.scene().worldBounds(), (fluir::editor::Rect{0, 0, 0, 0}));

  const std::vector<std::string> texts = textStrings(renderer.calls);
  EXPECT_EQ(std::find(texts.begin(), texts.end(), "main"), texts.end());
}

TEST(ModulePage, DeleteKeepsTheCurrentViewport) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kSimpleBinary;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  // A refit after the delete would move every survivor on screen.
  const Viewport v = fitViewport(ctx, *l.result.tree, renderer.outputSize_);
  const Rect survivorScreen = toScreenRect(v, Rect{60, 135, 25, 25});  // constant id=3

  page.draw();
  ASSERT_TRUE(hasRect(renderer.calls, survivorScreen));
  renderer.calls.clear();

  page.update({mouseDown(InputEvent::Button::Left, v.worldToScreen(kInsideConstant2)), deleteKey()});
  page.draw();

  EXPECT_TRUE(hasRect(renderer.calls, survivorScreen));
}

TEST(ModulePage, DeleteDuringADragDoesNotCrash) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  // The rebuild frees every actor, so the in-flight drag must be dropped first.
  const Viewport v = fitViewport(ctx, *l.result.tree, renderer.outputSize_);
  const Vec2 grip = v.worldToScreen(kOnDragHandle);
  page.update({mouseDown(InputEvent::Button::Left, grip),
               mouseMove(grip + Vec2{10, 0}),
               deleteKey(),
               mouseMove(grip + Vec2{20, 0}),
               mouseUp(InputEvent::Button::Left, grip + Vec2{20, 0})});
  page.draw();

  EXPECT_EQ(page.scene().find(1, 1), nullptr);
}

TEST(ModulePage, DeleteThenSavePersistsTheRemoval) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const fs::path tmp = fs::temp_directory_path() / "fluir_module_delete_test.fl";
  fs::copy_file(kIntConstants, tmp, fs::copy_options::overwrite_existing);

  EditorContext ctx;
  ctx.program = tmp;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);
  page.draw();

  // The body click opens this i8 constant's in-place editor, which owns the
  // keyboard; Escape closes it so Delete is a page command again.
  page.update({mouseDown(InputEvent::Button::Left, toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideActor)),
               keyDown(InputEvent::Key::Escape),
               deleteKey()});
  page.update({mouseDown(InputEvent::Button::Left, page.header().saveButton().bounds().center())});

  fluir::editor::CollectingSink sink;
  const auto reloaded = reloadFrom(sink, tmp);
  ASSERT_TRUE(reloaded.tree.has_value());

  const auto& fn = std::get<fluir::pt::FunctionDecl>(reloaded.tree->declarations.at(1));
  EXPECT_FALSE(fn.body.nodes.contains(1));
  EXPECT_TRUE(fn.body.nodes.contains(2));

  fs::remove(tmp);
}

TEST(ModulePage, DeleteAfterADragKeepsTheDraggedPosition) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  // Without a syncTreeFromScene() before the edit, the rebuild reverts this drag.
  const Viewport v = fitViewport(ctx, *l.result.tree, renderer.outputSize_);
  const Vec2 grip = v.worldToScreen(kOnDragHandle);
  const Vec2 gripEnd = v.worldToScreen(kOnDragHandle + Vec2{50, 0});
  page.update(
    {mouseDown(InputEvent::Button::Left, grip), mouseMove(gripEnd), mouseUp(InputEvent::Button::Left, gripEnd)});
  ASSERT_EQ(page.scene().find(1, 1)->worldBounds().x, kActorWorldRect.x + 50);

  // Select and delete a *different* node: constant id=4, world {210,190,25,25}.
  // Escape closes the editor the body click opened, freeing the Delete key.
  page.update({mouseDown(InputEvent::Button::Left, v.worldToScreen(Vec2{213, 208})),
               keyDown(InputEvent::Key::Escape),
               deleteKey()});
  ASSERT_EQ(page.scene().find(1, 4), nullptr);

  ASSERT_NE(page.scene().find(1, 1), nullptr);
  EXPECT_EQ(page.scene().find(1, 1)->worldBounds().x, kActorWorldRect.x + 50);
}

TEST(ModulePage, DeleteOfAConduitSourcePersistsTheConduitRemoval) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const fs::path tmp = fs::temp_directory_path() / "fluir_module_delete_conduit_test.fl";
  fs::copy_file(kSimpleBinary, tmp, fs::copy_options::overwrite_existing);

  EditorContext ctx;
  ctx.program = tmp;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);
  page.draw();

  page.update(
    {mouseDown(InputEvent::Button::Left, toScreen(ctx, *l.result.tree, renderer.outputSize_, kInsideConstant2)),
     deleteKey()});
  page.update({mouseDown(InputEvent::Button::Left, page.header().saveButton().bounds().center())});

  fluir::editor::CollectingSink sink;
  const auto reloaded = reloadFrom(sink, tmp);
  ASSERT_TRUE(reloaded.tree.has_value());

  const auto& fn = std::get<fluir::pt::FunctionDecl>(reloaded.tree->declarations.at(1));
  EXPECT_FALSE(fn.body.nodes.contains(2));
  EXPECT_FALSE(fn.body.conduits.contains(4));  // conduit sourced from constant id=2
  EXPECT_TRUE(fn.body.conduits.contains(5));   // the sibling wire survives

  fs::remove(tmp);
}

TEST(ModulePage, ADragIsRecordedAsOneUndoableEdit) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);
  ASSERT_FALSE(page.editor().canUndo());

  const Viewport v = fitViewport(ctx, *l.result.tree, renderer.outputSize_);
  const Vec2 grip = v.worldToScreen(kOnDragHandle);
  const Vec2 gripEnd = v.worldToScreen(kOnDragHandle + Vec2{50, 0});
  page.update(
    {mouseDown(InputEvent::Button::Left, grip), mouseMove(gripEnd), mouseUp(InputEvent::Button::Left, gripEnd)});

  EXPECT_TRUE(page.editor().canUndo()) << "the whole gesture is one undoable edit";
}

TEST(ModulePage, UndoAfterADragRestoresTheOriginalPosition) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  const Viewport v = fitViewport(ctx, *l.result.tree, renderer.outputSize_);
  const Vec2 grip = v.worldToScreen(kOnDragHandle);
  const Vec2 gripEnd = v.worldToScreen(kOnDragHandle + Vec2{50, 0});
  page.update(
    {mouseDown(InputEvent::Button::Left, grip), mouseMove(gripEnd), mouseUp(InputEvent::Button::Left, gripEnd)});
  ASSERT_EQ(page.scene().find(1, 1)->worldBounds().x, kActorWorldRect.x + 50);

  ASSERT_TRUE(page.editor().undo());
  page.scene().layout(ctx);

  EXPECT_EQ(page.scene().find(1, 1)->worldBounds(), kActorWorldRect);
}

TEST(ModulePage, RedoAfterUndoingADragReturnsTheNodeToTheDroppedPosition) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  const Viewport v = fitViewport(ctx, *l.result.tree, renderer.outputSize_);
  const Vec2 grip = v.worldToScreen(kOnDragHandle);
  const Vec2 gripEnd = v.worldToScreen(kOnDragHandle + Vec2{50, 0});
  page.update(
    {mouseDown(InputEvent::Button::Left, grip), mouseMove(gripEnd), mouseUp(InputEvent::Button::Left, gripEnd)});

  ASSERT_TRUE(page.editor().undo());
  ASSERT_TRUE(page.editor().redo());
  page.scene().layout(ctx);

  EXPECT_EQ(page.scene().find(1, 1)->worldBounds().x, kActorWorldRect.x + 50);
}

TEST(ModulePage, ADragThatEndsWhereItStartedRecordsNothing) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  const Viewport v = fitViewport(ctx, *l.result.tree, renderer.outputSize_);
  const Vec2 grip = v.worldToScreen(kOnDragHandle);
  page.update({mouseDown(InputEvent::Button::Left, grip), mouseUp(InputEvent::Button::Left, grip)});

  EXPECT_FALSE(page.editor().canUndo());
  EXPECT_EQ(page.scene().find(1, 1)->worldBounds(), kActorWorldRect);
}

TEST(ModulePage, UndoOfADragDoesNotDisturbTheSelection) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  const Viewport v = fitViewport(ctx, *l.result.tree, renderer.outputSize_);
  const Vec2 grip = v.worldToScreen(kOnDragHandle);
  const Vec2 gripEnd = v.worldToScreen(kOnDragHandle + Vec2{50, 0});
  page.update(
    {mouseDown(InputEvent::Button::Left, grip), mouseMove(gripEnd), mouseUp(InputEvent::Button::Left, gripEnd)});
  ASSERT_TRUE(page.scene().selected().has_value());

  ASSERT_TRUE(page.editor().undo());

  ASSERT_TRUE(page.scene().selected().has_value());
  EXPECT_EQ(*page.scene().selected(), (fluir::FullID{1, 1}));
}

TEST(ModulePage, UndoingADeleteMadeDuringADragRestoresTheOriginalPosition) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  EditorContext ctx;
  ctx.program = kIntConstants;
  RecordingRenderer renderer;
  ModulePage page{ctx, renderer};
  ASSERT_EQ(page.start(), 0);

  // Deleting mid-gesture must drop the preview, not leave it to reappear on undo.
  const Viewport v = fitViewport(ctx, *l.result.tree, renderer.outputSize_);
  const Vec2 grip = v.worldToScreen(kOnDragHandle);
  page.update({mouseDown(InputEvent::Button::Left, grip), mouseMove(v.worldToScreen(kOnDragHandle + Vec2{50, 0}))});
  page.update({deleteKey()});
  ASSERT_EQ(page.scene().find(1, 1), nullptr);

  ASSERT_TRUE(page.editor().undo());
  page.scene().layout(ctx);

  ASSERT_NE(page.scene().find(1, 1), nullptr);
  EXPECT_EQ(page.scene().find(1, 1)->worldBounds(), kActorWorldRect);
}

// In-place constant editing, routed by ModulePage's TextEditRouter: an open
// edit owns every key and text event, mouse presses only ever open/move/close
// it, and every existing gesture still runs.

namespace {

  /** A page on int_constants.fl, plus the fit transform its start() applied. */
  struct EditPage {
    EditorContext ctx;
    RecordingRenderer renderer;
    Loaded loaded = loadFixture("read/int_constants.fl");
    std::unique_ptr<ModulePage> page;
    Viewport view;

    explicit EditPage(const fs::path& program = kIntConstants) {
      ctx.program = program;
      page = std::make_unique<ModulePage>(ctx, renderer);
      EXPECT_TRUE(loaded.result.tree.has_value());
      EXPECT_EQ(page->start(), 0);
      view = fitViewport(ctx, *loaded.result.tree, renderer.outputSize_);
    }

    Vec2 at(Vec2 world) const { return view.worldToScreen(world); }
  };

}  // namespace

TEST(ModulePage, ClickingAConstantOpensItsEditor) {
  EditPage p;

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideActor))});

  const InlineEdit* field = fieldOf(*p.page, 1);
  ASSERT_NE(field, nullptr);
  EXPECT_TRUE(field->active());
  EXPECT_EQ(field->field()->text(), "-5") << "the draft is prefilled from the current value";
}

TEST(ModulePage, ClickingAConstantStillSelectsIt) {
  EditPage p;

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideActor))});

  ASSERT_TRUE(p.page->scene().selected().has_value());
  EXPECT_EQ(*p.page->scene().selected(), (fluir::FullID{1, 1}));
  EXPECT_TRUE(fieldOf(*p.page, 1)->active()) << "activation is additive, not a replacement";
}

TEST(ModulePage, TypingThenEnterCommitsTheNewConstantValue) {
  EditPage p;

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideActor)),
                  keyDown(InputEvent::Key::End),
                  textInput("7"),
                  keyDown(InputEvent::Key::Return)});

  EXPECT_EQ(i8Of(*p.page, 1), -57);
  EXPECT_FALSE(fieldOf(*p.page, 1)->active()) << "a successful commit closes the editor";
}

TEST(ModulePage, EnterOnAnInvalidDraftKeepsTheOldValueAndTheEditorOpen) {
  EditPage p;

  // -5999 is outside i8's range, so the validator rejects the draft.
  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideActor)),
                  keyDown(InputEvent::Key::End),
                  textInput("999"),
                  keyDown(InputEvent::Key::Return)});

  EXPECT_EQ(i8Of(*p.page, 1), -5);
  const InlineEdit* field = fieldOf(*p.page, 1);
  EXPECT_TRUE(field->active());
  EXPECT_TRUE(field->field()->invalid());
  EXPECT_EQ(field->field()->text(), "-5999");
  EXPECT_FALSE(p.page->editor().canUndo()) << "a rejected commit raises no edit";
}

TEST(ModulePage, EscapeCancelsTheEditAndLeavesTheValueAlone) {
  EditPage p;

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideActor)),
                  keyDown(InputEvent::Key::End),
                  textInput("7"),
                  keyDown(InputEvent::Key::Escape)});

  EXPECT_EQ(i8Of(*p.page, 1), -5);
  EXPECT_FALSE(fieldOf(*p.page, 1)->active());
  EXPECT_FALSE(p.page->editor().canUndo());
}

TEST(ModulePage, EscapeNoLongerStopsTheApp) {
  EditPage p;

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideActor)), keyDown(InputEvent::Key::Escape)});

  EXPECT_TRUE(p.ctx.running) << "Escape closes the editor, it is not an app command";
}

TEST(ModulePage, ClickingAnotherNodeCancelsTheOpenEdit) {
  EditPage p;

  p.page->update(
    {mouseDown(InputEvent::Button::Left, p.at(kInsideActor)), keyDown(InputEvent::Key::End), textInput("7")});
  ASSERT_TRUE(fieldOf(*p.page, 1)->active());

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideIntConstant2))});

  EXPECT_FALSE(fieldOf(*p.page, 1)->active());
  EXPECT_EQ(i8Of(*p.page, 1), -5) << "the abandoned draft raises nothing";
  EXPECT_TRUE(fieldOf(*p.page, 2)->active()) << "the newly clicked constant opens instead";
}

TEST(ModulePage, ClickingTheBackgroundCancelsTheOpenEdit) {
  EditPage p;

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideActor)), textInput("7")});
  ASSERT_TRUE(fieldOf(*p.page, 1)->active());

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kOutsideEveryActor))});

  EXPECT_FALSE(fieldOf(*p.page, 1)->active());
  EXPECT_EQ(i8Of(*p.page, 1), -5);
}

TEST(ModulePage, ClickingTheHeaderBarCancelsTheOpenEdit) {
  EditPage p;
  p.page->draw();  // lays the chrome bar out

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideActor)), textInput("7")});
  ASSERT_TRUE(fieldOf(*p.page, 1)->active());

  // Inside the bar but clear of every button (they are inset by textPad).
  const Rect bar = p.page->header().bounds();
  ASSERT_GT(bar.w, 0.0);
  p.page->update({mouseDown(InputEvent::Button::Left, Vec2{bar.x + 1, bar.y + 1})});

  EXPECT_FALSE(fieldOf(*p.page, 1)->active());
  EXPECT_EQ(i8Of(*p.page, 1), -5);
}

TEST(ModulePage, DeleteWhileEditingEditsTheDraftInsteadOfDeletingTheNode) {
  EditPage p;

  p.page->update(
    {mouseDown(InputEvent::Button::Left, p.at(kInsideActor)), keyDown(InputEvent::Key::Home), deleteKey()});

  ASSERT_NE(p.page->scene().find(1, 1), nullptr) << "Delete must not reach deleteSelection";
  const InlineEdit* field = fieldOf(*p.page, 1);
  ASSERT_TRUE(field->active());
  EXPECT_EQ(field->field()->text(), "5") << "Delete erased the character at the caret";
}

TEST(ModulePage, FWhileEditingTypesIntoTheDraftInsteadOfFittingTheView) {
  EditPage p;

  // Pan away from the fitted view first, so a stray fit would be visible.
  const Vec2 anchor{400, 300};
  const Vec2 shift{60, 40};
  p.page->update({mouseDown(InputEvent::Button::Middle, anchor),
                  mouseMove(anchor + shift),
                  mouseUp(InputEvent::Button::Middle, anchor + shift)});

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideActor) + shift), keyDown(InputEvent::Key::End)});
  ASSERT_TRUE(fieldOf(*p.page, 1)->active());

  // SDL keeps text input on, so an 'f' arrives as both a KeyDown and a TextInput.
  p.page->update({keyDown(InputEvent::Key::F), textInput("f")});

  EXPECT_EQ(fieldOf(*p.page, 1)->field()->text(), "-5f");

  const Rect fitted = toScreenRect(p.view, kActorWorldRect);
  const Rect panned{fitted.x + shift.x, fitted.y + shift.y, fitted.w, fitted.h};
  p.renderer.calls.clear();
  p.page->draw();
  EXPECT_TRUE(hasRect(p.renderer.calls, panned)) << "F must not refit the view mid-edit";
}

TEST(ModulePage, SpaceWhileEditingTypesIntoTheDraft) {
  EditPage p;

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideActor)), keyDown(InputEvent::Key::End)});
  ASSERT_TRUE(fieldOf(*p.page, 1)->active());

  p.page->update({keyDown(InputEvent::Key::Space), textInput(" ")});
  EXPECT_EQ(fieldOf(*p.page, 1)->field()->text(), "-5 ");

  // Nothing pans on a Left drag, Space or no Space.
  const Vec2 outside = p.at(kOutsideEveryActor);
  p.page->update({mouseDown(InputEvent::Button::Left, outside), mouseMove(outside + Vec2{50, 50})});
  p.renderer.calls.clear();
  p.page->draw();

  EXPECT_TRUE(hasRect(p.renderer.calls, toScreenRect(p.view, kActorWorldRect)));
}

// A middle-drag pan closes the edit (MouseDown non-Left cancels); the wheel does
// not, so zoom is what this pins.
TEST(ModulePage, ZoomingWhileEditingKeepsTheEditorOpen) {
  EditPage p;

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideActor)), keyDown(InputEvent::Key::End)});
  ASSERT_TRUE(fieldOf(*p.page, 1)->active());

  const Rect fitted = toScreenRect(p.view, kActorWorldRect);
  p.renderer.calls.clear();
  p.page->draw();
  ASSERT_TRUE(hasRect(p.renderer.calls, fitted));

  // The field re-derives its screen origin every frame, so a view change need
  // not close it; only key and text events are the editor's.
  p.page->update({mouseMove(p.at(kInsideActor) + Vec2{5, 5}), wheel(Vec2{400, 300}, Vec2{0, 1})});

  p.renderer.calls.clear();
  p.page->draw();
  EXPECT_FALSE(hasRect(p.renderer.calls, fitted)) << "the wheel must reach the graph viewport and zoom it";

  const InlineEdit* field = fieldOf(*p.page, 1);
  ASSERT_NE(field, nullptr);
  EXPECT_TRUE(field->active());
  EXPECT_EQ(field->field()->text(), "-5");
}

// Space reaches a focused field as text, and is nobody's modifier.
TEST(ModulePage, SpaceLeftOverAConstantOpensItsEditor) {
  EditPage p;

  const Vec2 start = p.at(kInsideActor);
  p.page->update({keyDown(InputEvent::Key::Space), mouseDown(InputEvent::Button::Left, start)});

  ASSERT_NE(fieldOf(*p.page, 1), nullptr);
  EXPECT_TRUE(fieldOf(*p.page, 1)->active());

  // ...and the drag that follows is the node's, not the view's.
  const Rect fitted = toScreenRect(p.view, kActorWorldRect);
  p.page->update({mouseMove(start + Vec2{50, 50})});
  p.renderer.calls.clear();
  p.page->draw();
  EXPECT_TRUE(hasRect(p.renderer.calls, fitted)) << "the view is still where the fit left it";
}

TEST(ModulePage, PressingAGripMidEditClosesTheFieldAndStartsTheGesture) {
  EditPage p;

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideActor)), textInput("7")});
  ASSERT_TRUE(fieldOf(*p.page, 1)->active());

  const Vec2 grip = p.at(kOnDragHandle);
  p.page->update({mouseDown(InputEvent::Button::Left, grip),
                  mouseMove(grip + Vec2{50 * p.view.scale, 0}),
                  mouseUp(InputEvent::Button::Left, grip + Vec2{50 * p.view.scale, 0})});

  EXPECT_FALSE(fieldOf(*p.page, 1)->active()) << "the grip press is a gesture, not an edit";
  EXPECT_EQ(i8Of(*p.page, 1), -5) << "the abandoned draft raises nothing";
  EXPECT_EQ(p.page->scene().find(1, 1)->worldBounds().x, kActorWorldRect.x + 50);
}

TEST(ModulePage, CommittingAValueIsOneUndoableEdit) {
  EditPage p;
  ASSERT_FALSE(p.page->editor().canUndo());

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideActor)),
                  keyDown(InputEvent::Key::End),
                  textInput("7"),
                  keyDown(InputEvent::Key::Return)});
  ASSERT_TRUE(p.page->editor().canUndo());
  ASSERT_EQ(i8Of(*p.page, 1), -57);

  ASSERT_TRUE(p.page->editor().undo());
  EXPECT_EQ(i8Of(*p.page, 1), -5) << "one undo restores the whole typing session";
  EXPECT_FALSE(p.page->editor().canUndo());
}

TEST(ModulePage, DeletingTheEditedNodeDropsTheDraft) {
  EditPage p;

  // constant id=2 survives the delete and stays where the fit transform put it.
  const Rect survivor = toScreenRect(p.view, Rect{110, 180, 25, 25});

  // Pan off the fitted view first, so the F below has something to undo.
  p.page->update({mouseDown(InputEvent::Button::Middle, Vec2{400, 300}), mouseMove(Vec2{460, 340})});

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideActor) + Vec2{60, 40}), textInput("7")});
  ASSERT_TRUE(fieldOf(*p.page, 1)->active());

  // The actor the router is focused on goes away underneath it.
  ASSERT_TRUE(p.ctx.dispatch(std::make_unique<fluir::editor::DeleteTransaction>(fluir::FullID{1, 1})));
  ASSERT_EQ(p.page->scene().find(1, 1), nullptr);

  // A stale focus must release the keyboard: F fits the view again. No mouse
  // event may intervene, or its own cancel() would mask the dropped focus.
  p.page->update({keyDown(InputEvent::Key::F)});
  p.renderer.calls.clear();
  p.page->draw();
  EXPECT_TRUE(hasRect(p.renderer.calls, survivor)) << "keys reach the page again once the edited actor is gone";

  // The delete only detached the actor, so an undo hands the very same one back:
  // the dropped focus must not latch onto it and resume the abandoned draft.
  ASSERT_TRUE(p.page->editor().undo());
  p.page->scene().layout(p.ctx);
  ASSERT_NE(p.page->scene().find(1, 1), nullptr);
  EXPECT_EQ(i8Of(*p.page, 1), -5);

  const std::string draft = fieldOf(*p.page, 1)->field()->text();
  p.page->update({textInput("9"), keyDown(InputEvent::Key::Return)});
  EXPECT_EQ(fieldOf(*p.page, 1)->field()->text(), draft)
    << "the restored actor is not focused, so nothing types into it";
  EXPECT_EQ(i8Of(*p.page, 1), -5) << "and Return commits nothing";
}

TEST(ModulePage, SaveAfterAnEditPersistsTheNewValue) {
  const fs::path tmp = fs::temp_directory_path() / "fluir_module_constant_edit_test.fl";
  fs::copy_file(kIntConstants, tmp, fs::copy_options::overwrite_existing);

  EditPage p{tmp};
  p.page->draw();  // lays out header_/saveButton_ bounds

  p.page->update({mouseDown(InputEvent::Button::Left, p.at(kInsideActor)),
                  keyDown(InputEvent::Key::End),
                  textInput("7"),
                  keyDown(InputEvent::Key::Return)});
  ASSERT_EQ(i8Of(*p.page, 1), -57);

  p.page->update({mouseDown(InputEvent::Button::Left, p.page->header().saveButton().bounds().center())});

  fluir::editor::CollectingSink sink;
  const auto reloaded = reloadFrom(sink, tmp);
  ASSERT_TRUE(reloaded.tree.has_value());

  const auto& fn = std::get<fluir::pt::FunctionDecl>(reloaded.tree->declarations.at(1));
  const auto& node = std::get<fluir::pt::Constant>(fn.body.nodes.at(1));
  EXPECT_EQ(std::get<fluir::literals_types::I8>(node.value), -57);

  fs::remove(tmp);
}
