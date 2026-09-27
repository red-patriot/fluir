#include "editor/tools/port_drag_tool.hpp"

#include <algorithm>
#include <optional>
#include <utility>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/view/draw/conditional.hpp"
#include "editor/view/graph_layout.hpp"
#include "tool_harness.hpp"

namespace {

  using fluir::FlowGraphLocation;
  using fluir::FullID;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::InputEvent;
  using fluir::editor::PortDragTool;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::key;
  using testutil::move;
  using testutil::up;

  const EditorContext kCtx;
  const FullID kConditional{1, 20};
  constexpr int kStartY = 8;
  constexpr double kUnit = 5;  // kCtx.layout.unitPx

  fluir::editor::et::ParseTree conditionalTree() {
    fluir::editor::et::FunctionDecl fn;
    fn.id = 1;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    fn.body.nodes.emplace(
      20,
      fluir::editor::et::Conditional{.id = 20,
                                     .location = {.x = 2, .y = 2, .z = 0, .width = 20, .height = 18},
                                     .condition = {.innerId = 1, .y = kStartY},
                                     .inputs = {},
                                     .outputs = {},
                                     .thenScope = xyz::indirect<fluir::editor::et::Block>{},
                                     .elseScope = xyz::indirect<fluir::editor::et::Block>{}});

    fluir::editor::et::ParseTree tree;
    tree.declarations.emplace(1, fluir::editor::et::Declaration{std::move(fn)});
    return tree;
  }

  struct Harness {
    EditorState state{kCtx};
    PortDragTool tool;

    Harness() { state.editor.load(std::nullopt, conditionalTree()); }

    bool send(const InputEvent& event) { return testutil::send(tool, state, event); }
    const fluir::editor::et::Conditional& conditional() const {
      return std::get<fluir::editor::et::Conditional>(*fluir::editor::nodeAt(state.editor.tree(), kConditional));
    }
    int portY() const { return conditional().condition.y; }

    // The port's body, clear of the terminal dots on its edge midpoints.
    Vec2 grab() const {
      const auto boxes = fluir::editor::layoutGraph(state.editor.tree(), kCtx.layout);
      const auto frame = std::ranges::find_if(
        boxes, [](const auto& b) { return b.path == kConditional && b.part == fluir::editor::Part::Frame; });
      const Rect port = fluir::editor::draw::conditionPortRect(conditional(), frame->world, kCtx.layout);
      return Vec2{port.x + port.w / 2, port.y + 1};
    }
  };

}  // namespace

TEST(PortDragTool, OnlyAPressOnAPortClaimsTheGesture) {
  Harness h;

  EXPECT_FALSE(h.send(down(Vec2{60, 100})));  // the branch, beside the port
  EXPECT_FALSE(h.tool.capturing());
  EXPECT_FALSE(h.send(down(h.grab(), InputEvent::Button::Right)));

  EXPECT_TRUE(h.send(down(h.grab())));
  EXPECT_TRUE(h.tool.capturing());
}

TEST(PortDragTool, ADragMovesThePortLiveInWholeUnitsAndIgnoresX) {
  Harness h;
  const Vec2 at = h.grab();
  const FlowGraphLocation before = *fluir::editor::locationAt(h.state.editor.tree(), kConditional);
  ASSERT_TRUE(h.send(down(at)));

  EXPECT_TRUE(h.send(move(at + Vec2{50, 2 * kUnit + 1})));

  EXPECT_EQ(h.portY(), kStartY + 2);
  EXPECT_EQ(*fluir::editor::locationAt(h.state.editor.tree(), kConditional), before) << "the conditional stays put";
  EXPECT_FALSE(h.state.editor.canUndo()) << "nothing is recorded until release";
}

TEST(PortDragTool, ThePortStopsAtTheHeaderBandAndTheBottom) {
  Harness h;
  const auto limits = fluir::editor::draw::portYLimits(h.conditional(), kCtx.layout);
  const Vec2 at = h.grab();
  ASSERT_TRUE(h.send(down(at)));

  h.send(move(at + Vec2{0, -1000}));
  EXPECT_EQ(h.portY(), limits.lower);
  h.send(move(at + Vec2{0, 1000}));
  EXPECT_EQ(h.portY(), limits.upper);
}

TEST(PortDragTool, ReleaseRecordsOneUndoableEdit) {
  Harness h;
  const auto before = h.state.editor.tree();
  const Vec2 at = h.grab();
  ASSERT_TRUE(h.send(down(at)));
  h.send(move(at + Vec2{0, kUnit}));
  h.send(move(at + Vec2{0, 3 * kUnit}));

  EXPECT_TRUE(h.send(up(at + Vec2{0, 3 * kUnit})));

  EXPECT_FALSE(h.tool.capturing());
  EXPECT_EQ(h.portY(), kStartY + 3);
  ASSERT_TRUE(h.state.editor.undo());
  EXPECT_EQ(h.state.editor.tree(), before);
  EXPECT_FALSE(h.state.editor.canUndo());
}

TEST(PortDragTool, AGestureThatEndsWhereItStartedRecordsNothing) {
  Harness h;
  const Vec2 at = h.grab();
  ASSERT_TRUE(h.send(down(at)));
  h.send(move(at + Vec2{0, 2 * kUnit}));
  h.send(move(at));

  h.send(up(at));

  EXPECT_EQ(h.portY(), kStartY);
  EXPECT_FALSE(h.state.editor.canUndo());
}

TEST(PortDragTool, EscapeRestoresThePort) {
  Harness h;
  const auto before = h.state.editor.tree();
  const Vec2 at = h.grab();
  ASSERT_TRUE(h.send(down(at)));
  h.send(move(at + Vec2{0, 2 * kUnit}));

  EXPECT_TRUE(h.send(key(InputEvent::Key::Escape)));

  EXPECT_FALSE(h.tool.capturing());
  EXPECT_EQ(h.state.editor.tree(), before);
  EXPECT_FALSE(h.state.editor.canUndo());
}

// Undo or delete under a live gesture must not crash or record a stale edit.
TEST(PortDragTool, ReleaseAfterThePortVanishedRecordsNothing) {
  Harness h;
  const Vec2 at = h.grab();
  ASSERT_TRUE(h.send(down(at)));
  h.send(move(at + Vec2{0, 2 * kUnit}));
  h.state.editor.tree().declarations.clear();

  h.send(up(at));

  EXPECT_FALSE(h.tool.capturing());
  EXPECT_FALSE(h.state.editor.canUndo());
}
