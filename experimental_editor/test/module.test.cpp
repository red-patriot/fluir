#include "editor/pages/module.hpp"

#include <algorithm>
#include <cmath>
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
#include "compiler/models/operator.hpp"
#include "editor/components/menu.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"
#include "editor/tools/completion_modal.hpp"
#include "editor/tools/menu_popup.hpp"
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
  constexpr Vec2 kConstant1Text{76, 177};  // value region after "i8", above the grip
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
  h.send({key(InputEvent::Key::Escape)});  // close the f64 draft so Delete reaches the page

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
  h.send({key(InputEvent::Key::Escape)});  // close the f64 draft so Delete reaches the page
  const Viewport before = h.page->state().view;

  h.send({key(InputEvent::Key::Delete)});

  EXPECT_EQ(nodeAt(h.tree(), FullID{1, 2}), nullptr);

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

namespace {

  // Binary 1's body clear of its grip, and where the page anchors its operator menu.
  constexpr Vec2 kBinaryBody{127, 107};
  const FullID kBinary{1, 1};

  fluir::Operator op(const Harness& h) { return std::get<fluir::pt::Binary>(*nodeAt(h.tree(), kBinary)).op; }

  // The screen rect of the open menu's row labeled `label`.
  Rect menuRow(const Harness& h, const std::string& label, Rect worldAnchor = Rect{125, 85, 25, 25}) {
    const auto* menu = dynamic_cast<const fluir::editor::MenuPopup*>(h.page->state().popup.get());
    EXPECT_NE(menu, nullptr);
    if (menu == nullptr) {
      return {};
    }
    const Vec2 tl = h.screen(worldAnchor.topLeft());
    const double scale = h.page->state().view.scale;
    const Rect anchor{tl.x, tl.y, worldAnchor.w * scale, worldAnchor.h * scale};
    const Rect bounds{0, 0, h.renderer.outputSize_.x, h.renderer.outputSize_.y};
    const auto layout = fluir::editor::layoutMenu(menu->labels(), anchor, bounds, h.ctx.layout, h.page->state().text);
    const auto it = std::ranges::find(menu->labels(), label);
    EXPECT_NE(it, menu->labels().end()) << label;
    return layout.items.at(static_cast<std::size_t>(it - menu->labels().begin()));
  }

}  // namespace

TEST(ModulePage, PickingFromTheOperatorMenuIsOneUndoableEdit) {
  Harness h{kSimpleBinary};
  h.press(kBinaryBody);

  const Vec2 star = menuRow(h, "*").center();
  h.send({move(star), down(star), up(star)});

  EXPECT_EQ(op(h), fluir::Operator::STAR);
  EXPECT_EQ(h.page->state().popup, nullptr);
  h.click("Undo");
  EXPECT_EQ(op(h), fluir::Operator::PLUS);
}

TEST(ModulePage, APressOutsideTheOperatorMenuOnlyClosesIt) {
  Harness h{kSimpleBinary};
  h.press(kBinaryBody);
  ASSERT_NE(h.page->state().popup, nullptr);

  h.press(kConstant2Body);

  EXPECT_EQ(h.page->state().popup, nullptr);
  EXPECT_EQ(op(h), fluir::Operator::PLUS);
  EXPECT_EQ(h.page->state().selection, kBinary) << "the closing press is swallowed";
}

TEST(ModulePage, DeleteWhileTheOperatorMenuIsOpenDeletesNothing) {
  Harness h{kSimpleBinary};
  h.press(kBinaryBody);

  h.send({key(InputEvent::Key::Delete)});

  EXPECT_NE(nodeAt(h.tree(), kBinary), nullptr);
  EXPECT_NE(h.page->state().popup, nullptr);
}

TEST(ModulePage, APressOnTheHeaderBarClosesTheOperatorMenu) {
  Harness h{kSimpleBinary};
  h.press(kBinaryBody);
  ASSERT_NE(h.page->state().popup, nullptr);

  const Vec2 emptyBar{h.renderer.outputSize_.x / 2, h.ctx.layout.chromeHeaderPx / 2};
  h.send({down(emptyBar), up(emptyBar)});

  EXPECT_EQ(h.page->state().popup, nullptr);
}

namespace {

  // Index of the first fill of `frame`, or calls.size().
  std::size_t fillIndex(const std::vector<testutil::DrawCall>& calls, Rect frame) {
    const auto it = std::ranges::find_if(calls, [frame](const testutil::DrawCall& c) {
      return c.op == testutil::DrawCall::Op::Fill && std::abs(c.rect.x - frame.x) < 1e-6 &&
             std::abs(c.rect.y - frame.y) < 1e-6 && std::abs(c.rect.w - frame.w) < 1e-6 &&
             std::abs(c.rect.h - frame.h) < 1e-6;
    });
    return static_cast<std::size_t>(it - calls.begin());
  }

  // Clips still open just before `calls[index]`.
  int openClipsBefore(const std::vector<testutil::DrawCall>& calls, std::size_t index) {
    int open = 0;
    for (std::size_t i = 0; i < index && i < calls.size(); ++i) {
      open += calls[i].op == testutil::DrawCall::Op::PushClip ? 1 : 0;
      open -= calls[i].op == testutil::DrawCall::Op::PopClip ? 1 : 0;
    }
    return open;
  }

}  // namespace

TEST(ModulePage, DrawShowsTheOpenOperatorMenu) {
  Harness h{kSimpleBinary};
  h.press(kBinaryBody);

  ASSERT_EQ(h.page->draw(), 0);

  const auto texts = testutil::textStrings(h.renderer.calls);
  EXPECT_NE(std::find(texts.begin(), texts.end(), "&&"), texts.end());
  const auto* menu = dynamic_cast<const fluir::editor::MenuPopup*>(h.page->state().popup.get());
  ASSERT_NE(menu, nullptr);
  const Rect first = menuRow(h, menu->labels().front());
  const Rect menuFrame{first.x, first.y, first.w, first.h * static_cast<double>(menu->labels().size())};
  const std::size_t menuAt = fillIndex(h.renderer.calls, menuFrame);
  ASSERT_LT(menuAt, h.renderer.calls.size());
  EXPECT_LT(fillIndex(h.renderer.calls, Rect{0, 0, 800, h.ctx.layout.chromeHeaderPx}), menuAt);
  EXPECT_EQ(openClipsBefore(h.renderer.calls, menuAt), 0);
}

namespace {

  // function_with_input_only.fl: param a (id 2, I32) rail {50,75,75,25}; its tag, then its name.
  const fs::path kInputOnly = fs::path(TEST_FOLDER) / "read/function_with_input_only.fl";
  const Rect kParamARail{50, 75, 75, 25};
  constexpr Vec2 kParamATag{55, 90};

  const fluir::pt::FunctionDecl::Parameter& paramA(const Harness& h) {
    return fluir::editor::functionAt(h.tree(), FullID{1})->input->parameters.at(0);
  }

}  // namespace

TEST(ModulePage, PickingFromTheTypeMenuIsOneUndoableEdit) {
  Harness h{kInputOnly};
  h.press(kParamATag);

  const Vec2 f64 = menuRow(h, "F64", kParamARail).center();
  h.send({move(f64), down(f64), up(f64)});

  EXPECT_EQ(paramA(h).typeName, "F64");
  EXPECT_EQ(h.page->state().popup, nullptr);
  h.click("Undo");
  EXPECT_EQ(paramA(h).typeName, "I32");
}

TEST(ModulePage, APressOnATypeTagOpensNoNameDraft) {
  Harness h{kInputOnly};

  h.press(kParamATag);
  h.send({text("x"), key(InputEvent::Key::Return)});

  EXPECT_NE(h.page->state().popup, nullptr);
  EXPECT_EQ(paramA(h).name, "a");
}

namespace {

  // single_empty_function.fl: function 1 at world {50,50,500,500}; fit scale 1.2, pan {40,-60}.
  const fs::path kEmptyFunction = fs::path(TEST_FOLDER) / "read/single_empty_function.fl";
  constexpr Vec2 kFnBackground{700, 700};
  constexpr Vec2 kFnBody{60, 60};               // in the header
  constexpr Vec2 kFnBodyOutsideModal{60, 400};  // screen {112,420}, left of the modal

  void rightPress(Harness& h, Vec2 world) {
    h.send({down(h.screen(world), InputEvent::Button::Right), up(h.screen(world), InputEvent::Button::Right)});
  }

  const fluir::editor::CompletionModal* modal(const Harness& h) {
    return dynamic_cast<const fluir::editor::CompletionModal*>(h.page->state().popup.get());
  }

}  // namespace

TEST(ModulePage, ARightPressOnTheBackgroundOpensTheCompletionModal) {
  Harness h{kEmptyFunction};

  rightPress(h, kFnBackground);

  ASSERT_NE(modal(h), nullptr);
  ASSERT_EQ(h.page->draw(), 0);
  const auto texts = testutil::textStrings(h.renderer.calls);
  EXPECT_NE(std::ranges::find(texts, "Function"), texts.end());
  EXPECT_NE(std::ranges::find(texts, "Comment"), texts.end());
}

TEST(ModulePage, ARightPressOnAFunctionHeaderOpensNoModal) {
  Harness h{kEmptyFunction};

  rightPress(h, kFnBody);

  EXPECT_EQ(h.page->state().popup, nullptr);
}

TEST(ModulePage, ARightPressInsideAFunctionBodyOpensTheBodyCompletions) {
  Harness h{kEmptyFunction};

  rightPress(h, kFnBodyOutsideModal);

  ASSERT_NE(modal(h), nullptr);
  EXPECT_NE(std::ranges::find(modal(h)->labels(), "+ (binary)"), modal(h)->labels().end());
}

TEST(ModulePage, APressOutsideTheCompletionModalOnlyClosesIt) {
  Harness h{kEmptyFunction};
  rightPress(h, kFnBackground);
  ASSERT_NE(modal(h), nullptr);
  ASSERT_FALSE(modal(h)->frame().contains(h.screen(kFnBodyOutsideModal)));

  h.press(kFnBodyOutsideModal);

  EXPECT_EQ(h.page->state().popup, nullptr);
  EXPECT_FALSE(h.page->state().selection.has_value()) << "the closing press is swallowed";
}

TEST(ModulePage, DeleteWhileTheCompletionModalIsOpenDeletesNothing) {
  Harness h{kEmptyFunction};
  h.press(kFnBody);
  rightPress(h, kFnBackground);
  ASSERT_NE(modal(h), nullptr);

  h.send({key(InputEvent::Key::Delete)});

  EXPECT_FALSE(h.tree().declarations.empty());
  EXPECT_NE(h.page->state().popup, nullptr);
}

TEST(ModulePage, TheCompletionModalPaintsCenteredOverEverythingUnclipped) {
  Harness h{kEmptyFunction};
  const Vec2 at{400, 300};
  h.send({down(at, InputEvent::Button::Middle), move(at + Vec2{50, 30}), up(at, InputEvent::Button::Middle)});
  h.send({wheel(at, 1)});
  rightPress(h, kFnBackground);
  ASSERT_NE(modal(h), nullptr);

  const Rect frame = modal(h)->frame();
  EXPECT_NEAR(frame.w, 400, 1e-6);
  testutil::expectVecNear(frame.center(), Vec2{400, 300});

  ASSERT_EQ(h.page->draw(), 0);

  const auto& calls = h.renderer.calls;
  const std::size_t modalAt = fillIndex(calls, frame);
  ASSERT_LT(modalAt, calls.size());
  EXPECT_EQ(openClipsBefore(calls, modalAt), 0);
  EXPECT_LT(fillIndex(calls, Rect{0, 0, 800, h.ctx.layout.chromeHeaderPx}), modalAt);
  for (std::size_t i = modalAt; i < calls.size(); ++i) {
    if (calls[i].op == testutil::DrawCall::Op::PopClip) {
      continue;  // closes the modal's own row clip
    }
    const bool inFrame = calls[i].op == testutil::DrawCall::Op::Text ? frame.contains(calls[i].a) :
                                                                       frame.contains(calls[i].rect.topLeft());
    EXPECT_TRUE(inFrame) << "call " << i << " after the modal is not the modal's";
  }
}

namespace {

  // simple_binary_expr.fl: constant 2's output port, binary 1's input ports.
  constexpr Vec2 kConstant2Out{85, 97.5};
  constexpr Vec2 kBinaryIn0{125, 86};
  constexpr Vec2 kBinaryIn1{125, 110};
  // function_with_input_only.fl: param a's port, inside its name label.
  constexpr Vec2 kParamAPort{124, 87.5};

}  // namespace

TEST(ModulePage, APortToPortDragAddsAConduitAsOneUndoableEdit) {
  Harness h{kSimpleBinary};
  const fluir::pt::ParseTree before = h.tree();

  h.drag(kConstant2Out, kBinaryIn1 - kConstant2Out);

  const auto& conduits = fluir::editor::blockOf(h.tree(), FullID{1})->conduits;
  const std::vector<fluir::pt::Conduit::Output> toIn1{{.target = 1, .index = 1}};
  EXPECT_TRUE(std::ranges::any_of(
    conduits, [&](const auto& entry) { return entry.second.input == 2 && entry.second.children == toIn1; }));
  h.click("Undo");
  EXPECT_EQ(h.tree(), before);
}

TEST(ModulePage, APressOnAPortOpensNoOperatorMenu) {
  Harness h{kSimpleBinary};

  h.press(kBinaryIn0);

  EXPECT_EQ(h.page->state().popup, nullptr);
}

TEST(ModulePage, APressOnAParameterPortOpensNoNameDraft) {
  Harness h{kInputOnly};

  h.press(kParamAPort);
  h.send({text("x"), key(InputEvent::Key::Return)});

  EXPECT_EQ(paramA(h).name, "a");
}
