#include "editor/tools/conduit_tool.hpp"

#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/core/viewport.hpp"
#include "editor/view/graph_layout.hpp"
#include "recording_renderer.hpp"
#include "tool_harness.hpp"

// simple_binary_expr.fl: constant 2 {60,85,25,25} outputs at (85,97.5); constant 3 {60,135,25,25} at (85,147.5);
// binary 1 {125,85,25,25} takes inputs at (125,85) and (125,110). Conduit 5 feeds input 1 from constant 3.

namespace {

  using fluir::FullID;
  using fluir::ID;
  using fluir::editor::blockOf;
  using fluir::editor::Box;
  using fluir::editor::ConduitTool;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::InputEvent;
  using fluir::editor::layoutGraph;
  using fluir::editor::Rect;
  using fluir::editor::Subview;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::key;
  using testutil::move;
  using testutil::up;

  const EditorContext kCtx;
  constexpr Vec2 kConstant2Out{85, 97.5};
  constexpr Vec2 kConstant3Out{85, 147.5};
  constexpr Vec2 kConstant3Body{65, 140};
  constexpr Vec2 kBinaryIn1{125, 110};
  constexpr Vec2 kEmpty{300, 300};

  struct Harness {
    EditorState state{kCtx};
    ConduitTool tool;

    Harness() { testutil::loadInto(state, "read/simple_binary_expr.fl"); }

    bool send(const InputEvent& event) { return testutil::send(tool, state, event); }
    Vec2 screen(Vec2 world) const { return state.view.worldToScreen(world); }

    bool drag(Vec2 from, Vec2 to) {
      send(down(screen(from)));
      send(move(screen(to)));
      return send(up(screen(to)));
    }

    std::vector<testutil::DrawCall> draw() const {
      testutil::RecordingRenderer r;
      const std::vector<Box> boxes = layoutGraph(state.editor.tree(), kCtx.layout);
      {
        const Subview root{state.view, Rect{0, 0, 800, 600}, r};
        tool.draw(root, state, boxes);
      }
      return r.calls;
    }
  };

  // Whether a conduit in function 1 carries `source`'s output to `target`'s input `index`.
  bool connected(const fluir::pt::ParseTree& tree, ID source, ID target, int index) {
    for (const auto& [id, conduit] : blockOf(tree, FullID{1})->conduits) {
      for (const auto& out : conduit.children) {
        if (conduit.input == source && conduit.index == 0 && out.target == target && out.index == index) {
          return true;
        }
      }
    }
    return false;
  }

}  // namespace

TEST(ConduitTool, APressOffATerminalIsNotClaimed) {
  Harness h;

  EXPECT_FALSE(h.send(down(h.screen(kEmpty))));
  EXPECT_FALSE(h.send(down(h.screen(kConstant3Body))));
  EXPECT_FALSE(h.tool.capturing());
}

TEST(ConduitTool, APressOnATerminalClaimsAndCaptures) {
  Harness h;

  EXPECT_TRUE(h.send(down(h.screen(kConstant2Out))));
  EXPECT_TRUE(h.tool.capturing());
}

TEST(ConduitTool, DraggingAnOutputToAnInputAddsOneUndoableConduit) {
  Harness h;
  const fluir::pt::ParseTree before = h.state.editor.tree();

  EXPECT_TRUE(h.drag(kConstant2Out, kBinaryIn1));

  EXPECT_FALSE(h.tool.capturing());
  EXPECT_TRUE(connected(h.state.editor.tree(), 2, 1, 1));
  EXPECT_FALSE(connected(h.state.editor.tree(), 3, 1, 1)) << "the fed input is rerouted";
  ASSERT_TRUE(h.state.editor.undo());
  EXPECT_EQ(h.state.editor.tree(), before);
  EXPECT_FALSE(h.state.editor.canUndo());
}

TEST(ConduitTool, DraggingAnInputToAnOutputConnectsTheSameWay) {
  Harness h;

  EXPECT_TRUE(h.drag(kBinaryIn1, kConstant2Out));

  EXPECT_TRUE(connected(h.state.editor.tree(), 2, 1, 1));
}

TEST(ConduitTool, AReleaseOffACompatibleTerminalAddsNothing) {
  for (const Vec2 target : {kEmpty, kConstant3Out, kConstant3Body}) {
    Harness h;
    const fluir::pt::ParseTree before = h.state.editor.tree();

    EXPECT_TRUE(h.drag(kConstant2Out, target));

    EXPECT_FALSE(h.tool.capturing());
    EXPECT_EQ(h.state.editor.tree(), before);
    EXPECT_FALSE(h.state.editor.canUndo());
  }
}

TEST(ConduitTool, EscapeCancelsTheDrag) {
  Harness h;
  const fluir::pt::ParseTree before = h.state.editor.tree();
  ASSERT_TRUE(h.send(down(h.screen(kConstant2Out))));

  EXPECT_TRUE(h.send(key(InputEvent::Key::Escape)));

  EXPECT_FALSE(h.tool.capturing());
  EXPECT_FALSE(h.send(up(h.screen(kBinaryIn1))));
  EXPECT_EQ(h.state.editor.tree(), before);
}

TEST(ConduitTool, DrawsALineFromTheTerminalToTheCursorOnlyWhileDragging) {
  Harness h;
  EXPECT_EQ(testutil::countOf(h.draw(), testutil::DrawCall::Op::Line), 0u);
  ASSERT_TRUE(h.send(down(h.screen(kConstant2Out))));
  h.send(move(h.screen(kEmpty)));

  EXPECT_TRUE(testutil::hasLine(h.draw(), h.screen(kConstant2Out), h.screen(kEmpty)));

  h.send(up(h.screen(kEmpty)));
  EXPECT_EQ(testutil::countOf(h.draw(), testutil::DrawCall::Op::Line), 0u);
}

TEST(ConduitTool, TerminalsAreFoundThroughThePannedAndZoomedView) {
  Harness h;
  h.state.view.scale = 2.0;
  h.state.view.pan = Vec2{30, -10};

  EXPECT_TRUE(h.drag(kConstant2Out, kBinaryIn1));

  EXPECT_TRUE(connected(h.state.editor.tree(), 2, 1, 1));
}

namespace {

  fluir::pt::Scope makeScope(ID id, int y, int h) {
    return fluir::pt::Scope{.id = id, .location = {.x = 0, .y = y, .z = 0, .width = 20, .height = h}, .body = {}};
  }

  fluir::pt::Constant makeConstant(ID id, int x, int y) {
    return fluir::pt::Constant{.id = id,
                               .location = {.x = x, .y = y, .z = 0, .width = 10, .height = 10},
                               .value = fluir::literals_types::I32{0}};
  }

  fluir::pt::Unary makeUnary(ID id, int x, int y) {
    return fluir::pt::Unary{
      .id = id, .location = {.x = x, .y = y, .z = 0, .width = 8, .height = 5}, .lhs = 0, .op = fluir::Operator::BANG};
  }

  // Function 1 {0,0,500,500} holds conditional 20. Both branches hold a constant 1 and a
  // unary 2, so the same ids appear on either side of the divider.
  // Frame {10,35,100,120}; each branch gives up its own top 25 to its header.
  //   then content from y 60: constant 1 {15,65,50,50}, unary 2 {70,65,40,25}
  //   else content from y 120: constant 1 {15,125,50,50}, unary 2 {70,125,40,25}
  fluir::pt::ParseTree conditionalTree() {
    fluir::pt::Scope then = makeScope(0, 0, 12);
    then.body.nodes.emplace(1, makeConstant(1, 1, 1));
    then.body.nodes.emplace(2, makeUnary(2, 12, 1));

    fluir::pt::Scope else_ = makeScope(1, 12, 12);
    else_.body.nodes.emplace(1, makeConstant(1, 1, 1));
    else_.body.nodes.emplace(2, makeUnary(2, 12, 1));

    fluir::pt::FunctionDecl fn;
    fn.id = 1;
    fn.location = fluir::FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    fn.body.nodes.emplace(20,
                          fluir::pt::Conditional{.id = 20,
                                                 .location = {.x = 2, .y = 2, .z = 0, .width = 20, .height = 24},
                                                 .condition = {},
                                                 .inputs = {},
                                                 .outputs = {},
                                                 .thenScope = xyz::indirect{std::move(then)},
                                                 .elseScope = xyz::indirect{std::move(else_)}});

    fluir::pt::ParseTree tree;
    tree.declarations.emplace(1, fluir::pt::Declaration{std::move(fn)});
    return tree;
  }

  struct NestedHarness {
    EditorState state{kCtx};
    ConduitTool tool;

    NestedHarness() { state.editor.load(conditionalTree()); }

    bool send(const InputEvent& event) { return testutil::send(tool, state, event); }

    bool drag(Vec2 from, Vec2 to) {
      send(down(state.view.worldToScreen(from)));
      send(move(state.view.worldToScreen(to)));
      return send(up(state.view.worldToScreen(to)));
    }

    const fluir::pt::Block* branch(const FullID& path) const { return blockOf(state.editor.tree(), path); }
  };

  constexpr Vec2 kThenConstantOut{65, 90};
  constexpr Vec2 kThenUnaryIn{70, 77.5};
  constexpr Vec2 kElseUnaryIn{70, 137.5};

}  // namespace

TEST(ConduitTool, WiresWithinOneBranch) {
  NestedHarness h;

  h.drag(kThenConstantOut, kThenUnaryIn);

  EXPECT_FALSE(h.branch(FullID{1, 20, 0})->conduits.empty());
  EXPECT_TRUE(h.branch(FullID{1, 20, 1})->conduits.empty());
}

// Same-parent is same-branch now: the two branches share node ids but not a block.
TEST(ConduitTool, RefusesToWireAcrossTheDivider) {
  NestedHarness h;

  h.drag(kThenConstantOut, kElseUnaryIn);

  EXPECT_TRUE(h.branch(FullID{1, 20, 0})->conduits.empty());
  EXPECT_TRUE(h.branch(FullID{1, 20, 1})->conduits.empty());
}
