#include "editor/pages/module.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"
#include "editor/view/graph_layout.hpp"
#include "fixture_loader.hpp"
#include "recording_renderer.hpp"
#include "tool_harness.hpp"

// ModulePage end to end: events in through update(), observed through the
// state it edits, what it draws, and what it saves. Points are given in world
// space and converted through the page's live view.

namespace {

  namespace fs = std::filesystem;

  using fluir::FlowGraphLocation;
  using fluir::FullID;
  using fluir::editor::EditorContext;
  using fluir::editor::InputEvent;
  using fluir::editor::locationAt;
  using fluir::editor::ModulePage;
  using fluir::editor::nodeAt;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::down;
  using testutil::key;
  using testutil::move;
  using testutil::text;
  using testutil::up;

  const fs::path kIntConstants = fs::path(TEST_FOLDER) / "read/int_constants.fl";
  const fs::path kSimpleBinary = fs::path(TEST_FOLDER) / "read/simple_binary_expr.fl";

  // int_constants.fl: function 1 {50,50,500,500}; constant 1 (I8 -5) {60,175,25,25}
  // at body units (2,20), grip {65,180,15,15}, text origin {64,179}.
  const FullID kConstant1{1, 1};
  constexpr Vec2 kConstant1Body{62, 197};
  constexpr Vec2 kConstant1Grip{72.5, 187.5};
  constexpr Vec2 kConstant1Text{64, 179};
  constexpr Vec2 kEmptyFrame{400, 400};
  constexpr Vec2 kBackground{-1000, -1000};
  constexpr Vec2 kHeaderGrip{537.5, 62.5};

  // simple_binary_expr.fl: binary 1 {125,85,25,25} (grip {130,90,15,15}); constant 2
  // {60,85,25,25} feeds it via conduit 4; constant 3 via conduit 5.
  constexpr Vec2 kBinaryGrip{137, 97};
  constexpr Vec2 kConstant2Body{62, 103};

  // Fit scale is 1.2 on the 800x600 recorder; 12 world px is 2 whole units with room to spare.
  constexpr double kTwoUnits = 12;

  struct Harness {
    EditorContext ctx;
    testutil::RecordingRenderer renderer;
    std::unique_ptr<ModulePage> page;

    explicit Harness(const fs::path& program) {
      ctx.program = program;
      page = std::make_unique<ModulePage>(ctx, renderer);
      EXPECT_EQ(page->start(), 0);
    }

    Vec2 screen(Vec2 world) const { return page->state().view.worldToScreen(world); }
    void send(const std::vector<InputEvent>& events) { EXPECT_EQ(page->update(events), 0); }
    void press(Vec2 world) { send({down(screen(world)), up(screen(world))}); }

    void drag(Vec2 world, Vec2 worldDelta) {
      send({down(screen(world)), move(screen(world + worldDelta)), up(screen(world + worldDelta))});
    }

    void click(std::string_view label) {
      const auto& buttons = page->header().buttons;
      const auto it = std::ranges::find(buttons, label, &fluir::editor::Button::label);
      ASSERT_NE(it, buttons.end()) << label;
      const Vec2 at = page->headerLayout().buttons[static_cast<std::size_t>(it - buttons.begin())].center();
      send({down(at), up(at)});
    }

    const fluir::pt::ParseTree& tree() const { return page->state().editor.tree(); }
    FlowGraphLocation loc(const FullID& path) const { return *locationAt(tree(), path); }
    fluir::pt::Literal value(const FullID& path) const {
      return std::get<fluir::pt::Constant>(*nodeAt(tree(), path)).value;
    }
  };

  fs::path scratchCopy(const fs::path& fixture, const std::string& name) {
    const fs::path out = fs::temp_directory_path() / ("fluir_module_test_" + name + ".fl");
    fs::copy_file(fixture, out, fs::copy_options::overwrite_existing);
    return out;
  }

  InputEvent wheel(Vec2 pos, double y) { return {.type = InputEvent::Type::Wheel, .pos = pos, .wheel = {0, y}}; }

}  // namespace

TEST(ModulePage, AClickOnANodeSelectsItNotTheFrame) {
  Harness h{kIntConstants};

  h.press(kConstant1Body);

  EXPECT_EQ(h.page->state().selection, kConstant1);
}

TEST(ModulePage, AClickOnEmptyFrameAreaSelectsTheFrame) {
  Harness h{kIntConstants};

  h.press(kEmptyFrame);

  EXPECT_EQ(h.page->state().selection, (FullID{1}));
}

TEST(ModulePage, AClickOnTheBackgroundClearsTheSelection) {
  Harness h{kIntConstants};
  h.press(kConstant1Body);

  h.press(kBackground);

  EXPECT_FALSE(h.page->state().selection.has_value());
}

TEST(ModulePage, APressOnAGripAlsoSelectsTheNode) {
  Harness h{kIntConstants};

  h.press(kConstant1Grip);

  EXPECT_EQ(h.page->state().selection, kConstant1);
}

TEST(ModulePage, ALeftClickOnANodeDoesNotPan) {
  Harness h{kIntConstants};
  const Vec2 before = h.page->state().view.pan;

  h.send({down(h.screen(kConstant1Body)), move(h.screen(kConstant1Body) + Vec2{30, 20}), up(h.screen(kConstant1Body))});

  EXPECT_EQ(h.page->state().view.pan, before);
}

TEST(ModulePage, AMiddleDragOverANodeStillPans) {
  Harness h{kIntConstants};
  const Vec2 before = h.page->state().view.pan;
  const fluir::pt::ParseTree tree = h.tree();
  const Vec2 at = h.screen(kConstant1Body);

  h.send(
    {down(at, InputEvent::Button::Middle), move(at + Vec2{30, 20}), up(at + Vec2{30, 20}, InputEvent::Button::Middle)});

  testutil::expectVecNear(h.page->state().view.pan, before + Vec2{30, 20});
  EXPECT_EQ(h.tree(), tree);
}

TEST(ModulePage, AGripDragMovesTheNodeAsOneUndoableEdit) {
  Harness h{kIntConstants};

  h.drag(kConstant1Grip, Vec2{kTwoUnits, 0});
  EXPECT_EQ(h.loc(kConstant1).x, 4);

  h.click("Undo");
  EXPECT_EQ(h.loc(kConstant1).x, 2);
  h.click("Redo");
  EXPECT_EQ(h.loc(kConstant1).x, 4);
}

TEST(ModulePage, AFunctionDragCarriesItsNodesWhichStayDraggable) {
  Harness h{kIntConstants};
  // The header grip starts under the header bar; pan it into view first.
  const Vec2 at = h.screen(kEmptyFrame);
  h.send(
    {down(at, InputEvent::Button::Middle), move(at + Vec2{0, 100}), up(at + Vec2{0, 100}, InputEvent::Button::Middle)});

  h.drag(kHeaderGrip, Vec2{kTwoUnits, kTwoUnits});
  ASSERT_EQ(h.loc(FullID{1}).x, 12);

  h.drag(kConstant1Grip + Vec2{10, 10}, Vec2{kTwoUnits, 0});
  EXPECT_EQ(h.loc(kConstant1).x, 4);
}

TEST(ModulePage, TheExitButtonLeavesForTheSplashPage) {
  Harness h{kIntConstants};

  h.click("Exit");

  EXPECT_NE(h.page->next(), nullptr);
}

TEST(ModulePage, QuitStopsTheAppEvenOverANode) {
  Harness h{kIntConstants};

  h.send({down(h.screen(kConstant1Body)), InputEvent{.type = InputEvent::Type::Quit}});

  EXPECT_FALSE(h.ctx.running);
}

TEST(ModulePage, EscapeDoesNotStopTheApp) {
  Harness h{kIntConstants};

  h.send({key(InputEvent::Key::Escape)});

  EXPECT_TRUE(h.ctx.running);
}

TEST(ModulePage, FFitsTheViewAgain) {
  Harness h{kIntConstants};
  const Viewport fitted = h.page->state().view;
  const Vec2 at = h.screen(kEmptyFrame);
  h.send({down(at, InputEvent::Button::Middle), move(at + Vec2{40, 40}), up(at, InputEvent::Button::Middle)});
  ASSERT_NE(h.page->state().view.pan, fitted.pan);

  h.send({key(InputEvent::Key::F)});

  EXPECT_EQ(h.page->state().view.pan, fitted.pan);
  EXPECT_EQ(h.page->state().view.scale, fitted.scale);
}

TEST(ModulePage, SaveWritesTheEditedTreeToTheProgram) {
  Harness h{scratchCopy(kIntConstants, "save")};
  h.drag(kConstant1Grip, Vec2{kTwoUnits, 0});

  h.click("Save");

  const testutil::Loaded reloaded = testutil::loadFixture(h.ctx.program->string());
  ASSERT_TRUE(reloaded.result.tree.has_value());
  EXPECT_EQ(*reloaded.result.tree, h.tree());
  EXPECT_EQ(locationAt(*reloaded.result.tree, kConstant1)->x, 4);
}

TEST(ModulePage, DeleteWithNoSelectionChangesNothing) {
  Harness h{kSimpleBinary};
  const fluir::pt::ParseTree before = h.tree();

  h.send({key(InputEvent::Key::Delete)});

  EXPECT_EQ(h.tree(), before);
  EXPECT_FALSE(h.page->state().editor.canUndo());
}

TEST(ModulePage, DeleteRemovesTheSelectedNodeItsConduitsAndTheSelection) {
  Harness h{kSimpleBinary};
  h.press(kConstant2Body);
  ASSERT_EQ(h.page->state().selection, (FullID{1, 2}));

  h.send({key(InputEvent::Key::Delete)});

  const auto& body = std::get<fluir::pt::FunctionDecl>(h.tree().declarations.at(1)).body;
  EXPECT_FALSE(body.nodes.contains(2));
  EXPECT_FALSE(body.conduits.contains(4));
  EXPECT_TRUE(body.conduits.contains(5));
  EXPECT_FALSE(h.page->state().selection.has_value());
}

TEST(ModulePage, DeleteOfAFrameRemovesTheWholeFunction) {
  Harness h{kSimpleBinary};
  h.press(kEmptyFrame);

  h.send({key(InputEvent::Key::Delete)});

  EXPECT_TRUE(h.tree().declarations.empty());
}

TEST(ModulePage, DeleteKeepsTheCurrentViewport) {
  Harness h{kSimpleBinary};
  h.press(kConstant2Body);
  const Viewport before = h.page->state().view;

  h.send({key(InputEvent::Key::Delete)});

  EXPECT_EQ(h.page->state().view.pan, before.pan);
  EXPECT_EQ(h.page->state().view.scale, before.scale);
}

// Page commands cancel the live gesture first, so the tree never keeps a half-drag.
TEST(ModulePage, DeleteDuringADragCancelsTheDragFirst) {
  Harness h{kSimpleBinary};
  const FlowGraphLocation original = h.loc(FullID{1, 1});
  h.send({down(h.screen(kBinaryGrip)), move(h.screen(kBinaryGrip + Vec2{kTwoUnits, 0}))});

  h.send({key(InputEvent::Key::Delete), up(h.screen(kBinaryGrip))});

  EXPECT_EQ(nodeAt(h.tree(), FullID{1, 1}), nullptr);
  h.click("Undo");
  EXPECT_EQ(h.loc(FullID{1, 1}), original);
}

TEST(ModulePage, UndoMidDragRestoresTheOriginalPosition) {
  Harness h{kSimpleBinary};
  const fluir::pt::ParseTree before = h.tree();
  h.send({down(h.screen(kBinaryGrip)), move(h.screen(kBinaryGrip + Vec2{kTwoUnits, 0}))});

  h.click("Undo");
  h.send({move(h.screen(kBinaryGrip + Vec2{2 * kTwoUnits, 0})), up(h.screen(kBinaryGrip))});

  EXPECT_EQ(h.tree(), before);
  EXPECT_FALSE(h.page->state().editor.canUndo());
}

TEST(ModulePage, TypingThenReturnCommitsAConstantAsOneUndoableEdit) {
  Harness h{kIntConstants};
  h.press(kConstant1Text);

  h.send({key(InputEvent::Key::End), text("0"), key(InputEvent::Key::Return)});

  EXPECT_EQ(h.value(kConstant1), fluir::pt::Literal{fluir::literals_types::I8{-50}});
  h.click("Undo");
  EXPECT_EQ(h.value(kConstant1), fluir::pt::Literal{fluir::literals_types::I8{-5}});
}

TEST(ModulePage, APressOnTheHeaderBarCancelsTheOpenEdit) {
  Harness h{kIntConstants};
  h.press(kConstant1Text);
  h.send({key(InputEvent::Key::End), text("0")});

  const Vec2 emptyBar{h.renderer.outputSize_.x / 2, h.ctx.layout.chromeHeaderPx / 2};
  h.send({down(emptyBar), up(emptyBar), key(InputEvent::Key::Return)});

  EXPECT_EQ(h.value(kConstant1), fluir::pt::Literal{fluir::literals_types::I8{-5}});
}

TEST(ModulePage, AnOpenDraftTakesKeysBeforePageCommands) {
  Harness h{kIntConstants};
  h.press(kConstant1Text);
  const Viewport view = h.page->state().view;

  h.send({key(InputEvent::Key::End), key(InputEvent::Key::Delete), key(InputEvent::Key::F)});

  EXPECT_NE(nodeAt(h.tree(), kConstant1), nullptr) << "Delete edited the draft, not the graph";
  EXPECT_EQ(h.page->state().view.pan, view.pan) << "F did not refit";
}

TEST(ModulePage, ZoomingWhileEditingKeepsTheDraft) {
  Harness h{kIntConstants};
  h.press(kConstant1Text);

  h.send({wheel(Vec2{400, 300}, 1), key(InputEvent::Key::End), text("0"), key(InputEvent::Key::Return)});

  EXPECT_EQ(h.value(kConstant1), fluir::pt::Literal{fluir::literals_types::I8{-50}});
}

TEST(ModulePage, APressOnAGripMidEditClosesTheDraftAndStartsTheGesture) {
  Harness h{kIntConstants};
  h.press(kConstant1Text);

  h.drag(kConstant1Grip, Vec2{kTwoUnits, 0});
  h.send({text("0"), key(InputEvent::Key::Return)});

  EXPECT_EQ(h.loc(kConstant1).x, 4);
  EXPECT_EQ(h.value(kConstant1), fluir::pt::Literal{fluir::literals_types::I8{-5}});
}

TEST(ModulePage, DrawShowsTheGraphUnderTheHeaderBar) {
  Harness h{kIntConstants};

  ASSERT_EQ(h.page->draw(), 0);

  const Vec2 tl = h.screen(Vec2{60, 175});
  const double scale = h.page->state().view.scale;
  EXPECT_TRUE(testutil::hasRect(h.renderer.calls, Rect{tl.x, tl.y, 25 * scale, 25 * scale}, 1e-6));
  EXPECT_TRUE(testutil::hasFill(h.renderer.calls, Rect{0, 0, 800, h.ctx.layout.chromeHeaderPx}));
  const auto texts = testutil::textStrings(h.renderer.calls);
  EXPECT_NE(std::find(texts.begin(), texts.end(), "int_constants.fl"), texts.end());
}
