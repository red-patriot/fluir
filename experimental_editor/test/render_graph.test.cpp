#include "editor/core/render_graph.hpp"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/utility/context.hpp"
#include "editor/core/collecting_sink.hpp"
#include "editor/core/loader.hpp"
#include "fixture_loader.hpp"
#include "recording_renderer.hpp"

// These tests assert *what is drawn where*: which primitive (rect / fill /
// line / text / clip), at what geometry, with what text. They use the
// `testutil` attribute-lookup matchers (hasRect, hasFill, hasTextAt, hasLine,
// clipsCovering, fillsOfSize, countOf, textStrings) and must not depend on
// draw-call count as a stand-in for unrelated behavior, or on the emission
// order of unrelated primitives.

namespace {

  using testutil::Loaded;
  using testutil::loadFixture;

  namespace fs = std::filesystem;

  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::clipsCovering;
  using testutil::containsRect;
  using testutil::countOf;
  using testutil::DrawCall;
  using testutil::expectRectNear;
  using testutil::fillsOfSize;
  using testutil::hasFill;
  using testutil::hasLine;
  using testutil::hasRect;
  using testutil::hasTextAt;
  using testutil::RecordingRenderer;
  using testutil::textStrings;

  // Body coordinates are frame-origin + kHeaderH (25 world px): body content is
  // rendered below the header band, so every body-relative y is offset by +25.
  //

  const fluir::editor::EditorContext kCtx;

}  // namespace

TEST(RenderGraph, EmptyOfFunctionsEmitsNoFrames) {
  const Loaded l = loadFixture("read/top_level_comment_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(r.calls.empty());
}

TEST(RenderGraph, SingleEmptyFunctionFrameAndHeader) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasRect(r.calls, Rect{50, 50, 500, 500}));
  EXPECT_TRUE(hasFill(r.calls, Rect{50, 50, 500, 25}));
  EXPECT_TRUE(hasTextAt(r.calls, "foo", Vec2{54, 54}));
  EXPECT_EQ(textStrings(r.calls), (std::vector<std::string>{"foo"}));
  EXPECT_EQ(countOf(r.calls, DrawCall::Op::Line), 0u);

  // The body Subview clips exactly once to the body rect; every Subview push is
  // balanced by a pop (full-canvas `world` + frame + body for an empty function).
  EXPECT_EQ(clipsCovering(r.calls, Rect{50, 75, 500, 500}).size(), 1u);
  EXPECT_EQ(countOf(r.calls, DrawCall::Op::PushClip), countOf(r.calls, DrawCall::Op::PopClip));
}

TEST(RenderGraph, ParamRailFromInputOnly) {
  const Loaded l = loadFixture("read/function_with_input_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasRect(r.calls, Rect{50, 50, 500, 500}));  // frame
  EXPECT_TRUE(hasRect(r.calls, Rect{50, 75, 75, 25}));    // param a rail slot
  EXPECT_TRUE(hasRect(r.calls, Rect{50, 100, 75, 25}));   // param b rail slot
  EXPECT_TRUE(hasFill(r.calls, Rect{50, 50, 500, 25}));   // header

  const auto dots = fillsOfSize(r.calls, 6, 6);
  EXPECT_EQ(dots.size(), 2u);  // 2 param port dots, no others
  EXPECT_TRUE(containsRect(dots, Rect{122, 84.5, 6, 6}));
  EXPECT_TRUE(containsRect(dots, Rect{122, 109.5, 6, 6}));

  EXPECT_TRUE(hasTextAt(r.calls, "I32 a", Vec2{54, 79}));
  EXPECT_TRUE(hasTextAt(r.calls, "I32 b", Vec2{54, 104}));
  const auto texts = textStrings(r.calls);
  EXPECT_NE(std::find(texts.begin(), texts.end(), "add"), texts.end());
}

TEST(RenderGraph, ParamRailUsesContextMetrics) {
  const Loaded l = loadFixture("read/function_with_input_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  // paramUnits 18 * unitPx 5 -> paramW 90; railUnits 6 * unitPx 5 -> railStep 30.
  // Body origin y = frameOrigin (50) + headerH (25) = 75; rows step by railStep.
  fluir::editor::EditorContext ctx;
  ctx.layout.paramUnits = 18.0;
  ctx.layout.railUnits = 6.0;
  RecordingRenderer r;
  fluir::editor::renderGraph(ctx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasRect(r.calls, Rect{50, 75, 90, 30}));   // param a rail slot
  EXPECT_TRUE(hasRect(r.calls, Rect{50, 105, 90, 30}));  // param b rail slot
}

TEST(RenderGraph, ReturnRailFromOutputOnly) {
  const Loaded l = loadFixture("read/function_with_output_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasRect(r.calls, Rect{50, 50, 500, 500}));  // frame
  EXPECT_TRUE(hasRect(r.calls, Rect{525, 75, 25, 25}));   // return rail slot
  EXPECT_TRUE(hasFill(r.calls, Rect{50, 50, 500, 25}));   // header

  const auto dots = fillsOfSize(r.calls, 6, 6);
  EXPECT_EQ(dots.size(), 1u);  // 1 return port dot, no others
  EXPECT_TRUE(containsRect(dots, Rect{522, 84.5, 6, 6}));

  EXPECT_TRUE(hasTextAt(r.calls, "F64", Vec2{529, 79}));
  const auto texts = textStrings(r.calls);
  EXPECT_NE(std::find(texts.begin(), texts.end(), "getVal"), texts.end());
}

TEST(RenderGraph, DeterministicAcrossRuns) {
  const Loaded l = loadFixture("read/multiple_empty_functions.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer a;
  RecordingRenderer b;
  fluir::editor::renderGraph(kCtx, *l.result.tree, Viewport{}, a);
  fluir::editor::renderGraph(kCtx, *l.result.tree, Viewport{}, b);

  EXPECT_EQ(a.calls, b.calls);
  EXPECT_FALSE(a.calls.empty());

  // Each function's name label is drawn exactly once; order between them is
  // not a contract worth asserting.
  const auto texts = textStrings(a.calls);
  EXPECT_EQ(std::count(texts.begin(), texts.end(), "foo"), 1);
  EXPECT_EQ(std::count(texts.begin(), texts.end(), "bar"), 1);
  EXPECT_EQ(std::count(texts.begin(), texts.end(), "baz"), 1);
}

TEST(RenderGraph, ViewportIsApplied) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const Viewport vp{.pan = {100, 50}, .scale = 2.0};
  RecordingRenderer r;
  fluir::editor::renderGraph(kCtx, *l.result.tree, vp, r);

  // world frame (50,50,500,500) -> screen (50*2+100, 50*2+50, 500*2, 500*2).
  EXPECT_TRUE(hasRect(r.calls, Rect{200, 150, 1000, 1000}));
  // world header (50,50,500,25) -> screen (200,150,1000,50).
  EXPECT_TRUE(hasFill(r.calls, Rect{200, 150, 1000, 50}));
  // world text pos (54,54) -> screen (54*2+100, 54*2+50).
  EXPECT_TRUE(hasTextAt(r.calls, "foo", Vec2{208, 158}));
}

TEST(RenderGraph, UnitPxScalesFrameAndHeader) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  // Fixture: x=10 y=10 w=100 h=100 (logical). With unitPx = 10 the frame doubles
  // vs. the default (5) and the header band stays headerUnits (5) tall in logical
  // units -> 50 world px.
  fluir::editor::EditorContext ctx;
  ctx.layout.unitPx = 10.0;
  RecordingRenderer r;
  fluir::editor::renderGraph(ctx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasRect(r.calls, Rect{100, 100, 1000, 1000}));  // frame
  EXPECT_TRUE(hasFill(r.calls, Rect{100, 100, 1000, 50}));    // header
}

TEST(RenderGraph, ConstantNode) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasRect(r.calls, Rect{60, 175, 25, 25}));   // constant -5
  EXPECT_TRUE(hasRect(r.calls, Rect{110, 180, 25, 25}));  // constant 318
  EXPECT_TRUE(hasRect(r.calls, Rect{160, 185, 25, 25}));  // constant 324
  EXPECT_TRUE(hasRect(r.calls, Rect{210, 190, 25, 25}));  // constant -12

  EXPECT_TRUE(hasTextAt(r.calls, "-5", Vec2{64, 179}));
  EXPECT_TRUE(hasTextAt(r.calls, "318", Vec2{114, 184}));
  EXPECT_TRUE(hasTextAt(r.calls, "324", Vec2{164, 189}));
  EXPECT_TRUE(hasTextAt(r.calls, "-12", Vec2{214, 194}));
  const auto texts = textStrings(r.calls);
  for (const std::string& want : {std::string{"-5"}, std::string{"318"}, std::string{"324"}, std::string{"-12"}}) {
    EXPECT_NE(std::find(texts.begin(), texts.end(), want), texts.end());
  }

  EXPECT_TRUE(hasFill(r.calls, Rect{50, 50, 500, 25}));  // header

  // Every constant's single output dot, no input dots.
  const auto dots = fillsOfSize(r.calls, 6, 6);
  EXPECT_EQ(dots.size(), 4u);
  EXPECT_TRUE(containsRect(dots, Rect{82, 184.5, 6, 6}));
  EXPECT_TRUE(containsRect(dots, Rect{132, 189.5, 6, 6}));
  EXPECT_TRUE(containsRect(dots, Rect{182, 194.5, 6, 6}));
  EXPECT_TRUE(containsRect(dots, Rect{232, 199.5, 6, 6}));

  EXPECT_EQ(countOf(r.calls, DrawCall::Op::Line), 0u);
}

TEST(RenderGraph, BinaryNodeHasTwoInputsOneOutput) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasTextAt(r.calls, "+", Vec2{129, 89}));  // stringify(PLUS)

  EXPECT_TRUE(hasFill(r.calls, Rect{122, 82, 6, 6}));    // binary input 0, y-frac 0.0
  EXPECT_TRUE(hasFill(r.calls, Rect{122, 107, 6, 6}));   // binary input 1, y-frac 1.0
  EXPECT_TRUE(hasFill(r.calls, Rect{147, 94.5, 6, 6}));  // binary output, y-frac 0.5
}

TEST(RenderGraph, UnaryNodeHasOneInput) {
  const Loaded l = loadFixture("read/simple_unary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasTextAt(r.calls, "-", Vec2{129, 89}));  // unary op label

  EXPECT_TRUE(hasFill(r.calls, Rect{122, 94.5, 6, 6}));  // unary single input, y-frac 0.5
  EXPECT_TRUE(hasFill(r.calls, Rect{147, 94.5, 6, 6}));  // unary output

  // constant id=3 output-0 anchor -> unary id=7 input-0 anchor.
  EXPECT_TRUE(hasLine(r.calls, Vec2{85, 97.5}, Vec2{125, 97.5}));
}

TEST(RenderGraph, CallNodeArgsAndReturn) {
  {
    const Loaded l = loadFixture("read/function_call.fl");
    ASSERT_TRUE(l.result.tree.has_value());

    RecordingRenderer r;
    fluir::editor::renderGraph(kCtx, *l.result.tree, Viewport{}, r);

    EXPECT_TRUE(hasTextAt(r.calls, "add", Vec2{154, 79}));
    EXPECT_TRUE(hasTextAt(r.calls, "a", Vec2{154, 104}));
    EXPECT_TRUE(hasTextAt(r.calls, "b", Vec2{154, 129}));

    EXPECT_TRUE(hasFill(r.calls, Rect{147, 109.5, 6, 6}));  // call arg row 0 input
    EXPECT_TRUE(hasFill(r.calls, Rect{147, 134.5, 6, 6}));  // call arg row 1 input
    EXPECT_TRUE(hasFill(r.calls, Rect{207, 102, 6, 6}));    // call return output

    // conduit id=4 (constant id=1 output-0 -> call arg row 1 input); conduit
    // id=5 targets an out-of-range input index and is guarded out.
    EXPECT_EQ(countOf(r.calls, DrawCall::Op::Line), 1u);
    EXPECT_TRUE(hasLine(r.calls, Vec2{25, 37.5}, Vec2{150, 137.5}));
  }
  {
    const Loaded l = loadFixture("read/function_call_no_args_no_returns.fl");
    ASSERT_TRUE(l.result.tree.has_value());

    RecordingRenderer r;
    fluir::editor::renderGraph(kCtx, *l.result.tree, Viewport{}, r);

    EXPECT_TRUE(hasTextAt(r.calls, "doStuff", Vec2{29, 54}));

    EXPECT_TRUE(fillsOfSize(r.calls, 6, 6).empty());  // no args, no return -> no port dots
    EXPECT_EQ(countOf(r.calls, DrawCall::Op::Line), 0u);
  }
}

TEST(RenderGraph, WireEndpointsMatchPorts) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_EQ(countOf(r.calls, DrawCall::Op::Line), 2u);
  EXPECT_TRUE(hasLine(r.calls, Vec2{85, 97.5}, Vec2{125, 85}));    // constant id=2 -> binary input-0
  EXPECT_TRUE(hasLine(r.calls, Vec2{85, 147.5}, Vec2{125, 110}));  // constant id=3 -> binary input-1
}

TEST(RenderGraph, ClipWrapsBodyForFunctionWithNodes) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(kCtx, *l.result.tree, Viewport{}, r);

  // The body rect is clipped twice: once by the frame actor's body container,
  // once by the Subview GraphRenderer still opens for rails and conduits.
  EXPECT_FALSE(clipsCovering(r.calls, Rect{50, 75, 500, 500}).empty());
  EXPECT_EQ(countOf(r.calls, DrawCall::Op::PushClip), countOf(r.calls, DrawCall::Op::PopClip));
  EXPECT_TRUE(hasRect(r.calls, Rect{125, 85, 25, 25}));  // binary node body rect
}

TEST(RenderGraph, DeterministicWithBodyAndWires) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer a;
  RecordingRenderer b;
  fluir::editor::renderGraph(kCtx, *l.result.tree, Viewport{}, a);
  fluir::editor::renderGraph(kCtx, *l.result.tree, Viewport{}, b);

  EXPECT_EQ(a.calls, b.calls);
  EXPECT_FALSE(testutil::opsOf(a.calls, DrawCall::Op::Line).empty());
  EXPECT_FALSE(testutil::opsOf(a.calls, DrawCall::Op::PushClip).empty());
}

TEST(GraphBounds, EmptyTreeIsZero) {
  const Loaded l = loadFixture("read/top_level_comment_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  expectRectNear(fluir::editor::graphBounds(kCtx, *l.result.tree), Rect{0, 0, 0, 0});
}

TEST(GraphBounds, SingleFunctionIsItsFrame) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  // Fixture: x=10 y=10 w=100 h=100 (logical); world px = logical * UNIT_PX (5).
  // Same frame rect asserted by SingleEmptyFunctionFrameAndHeader above.
  expectRectNear(fluir::editor::graphBounds(kCtx, *l.result.tree), Rect{50, 50, 500, 500});
}

TEST(GraphBounds, MultipleFunctionsUnion) {
  const Loaded l = loadFixture("read/multiple_empty_functions.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  // Fixture frames (logical -> world px, * UNIT_PX == 5):
  //   foo x=10  y=10 w=100 h=100 -> world [ 50, 50 ..  550, 550]
  //   baz x=330 y=10 w=100 h=100 -> world [1650, 50 .. 2150, 550]
  //   bar x=210 y=10 w=50  h=70  -> world [1050, 50 .. 1300, 400]
  // union: min (50, 50), max (2150, 550) -> {50, 50, 2100, 500}.
  expectRectNear(fluir::editor::graphBounds(kCtx, *l.result.tree), Rect{50, 50, 2100, 500});
}

TEST(GraphBounds, ScalesWithUnitPx) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  // Fixture: x=10 y=10 w=100 h=100 (logical). With unitPx = 10, world px doubles
  // relative to the default (5): {100, 100, 1000, 1000}.
  fluir::editor::EditorContext ctx;
  ctx.layout.unitPx = 10.0;
  expectRectNear(fluir::editor::graphBounds(ctx, *l.result.tree), Rect{100, 100, 1000, 1000});
}
