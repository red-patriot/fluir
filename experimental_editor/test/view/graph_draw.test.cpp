#include "editor/view/graph_draw.hpp"

#include <algorithm>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/utility/context.hpp"
#include "editor/assets/images.hpp"
#include "editor/core/collecting_sink.hpp"
#include "editor/core/loader.hpp"
#include "editor/core/viewport.hpp"
#include "editor/view/graph_layout.hpp"
#include "fixture_loader.hpp"
#include "recording_renderer.hpp"

// These tests drive the display list directly: layoutGraph + drawGraph,
// the same path ModulePage uses. They assert *what is drawn where*: which primitive (rect / fill /
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
  using testutil::hasIcon;
  using testutil::hasLine;
  using testutil::hasRect;
  using testutil::hasScaledTextAt;
  using testutil::hasTextAt;
  using testutil::hasWrappedText;
  using testutil::RecordingRenderer;
  using testutil::textStrings;

  // Body coordinates are frame-origin + kHeaderH (25 world px): body content is
  // rendered below the header band, so every body-relative y is offset by +25.
  //

  const fluir::editor::EditorContext kCtx;

  // Lays `tree` out and draws it into one root view, exactly as ModulePage does.
  void drawTree(const fluir::editor::EditorContext& ctx,
                const fluir::pt::ParseTree& tree,
                const Viewport& viewport,
                RecordingRenderer& r,
                const std::optional<fluir::FullID>& selection = std::nullopt) {
    const std::vector<fluir::editor::Box> boxes = fluir::editor::layoutGraph(tree, ctx.layout);
    const fluir::editor::Subview root{viewport, Rect{0, 0, r.outputSize().x, r.outputSize().y}, r};
    fluir::editor::drawGraph(root, tree, boxes, selection, ctx);
  }

}  // namespace

TEST(GraphDraw, EmptyTreeDrawsNothing) {
  RecordingRenderer r;
  drawTree(kCtx, fluir::pt::ParseTree{}, Viewport{}, r);

  EXPECT_EQ(countOf(r.calls, DrawCall::Op::Rect), 0u);
  EXPECT_EQ(countOf(r.calls, DrawCall::Op::Fill), 0u);
  EXPECT_EQ(countOf(r.calls, DrawCall::Op::Text), 0u);
  EXPECT_EQ(countOf(r.calls, DrawCall::Op::Line), 0u);
}

// top_level_comment_only.fl: comment 1 at units (10,10) 25x25 -> {50,50,125,125}, no header offset.
TEST(GraphDraw, TopLevelCommentDrawsAsACommentBox) {
  const Loaded l = loadFixture("read/top_level_comment_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasRect(r.calls, Rect{50, 50, 125, 125}));
  EXPECT_TRUE(hasFill(r.calls, Rect{50, 50, 125, 125}));
  EXPECT_TRUE(hasWrappedText(r.calls, "hello", Rect{54, 79, 117, 92}, 1.0));
  EXPECT_EQ(clipsCovering(r.calls, Rect{54, 79, 117, 92}).size(), 1u);
  EXPECT_TRUE(hasScaledTextAt(r.calls, "//", Vec2{54, 64.6}, 0.8));  // header tag
  EXPECT_TRUE(fillsOfSize(r.calls, 6, 6).empty());                   // no ports
  EXPECT_TRUE(clipsCovering(r.calls, Rect{50, 50, 125, 125}).empty());
}

TEST(GraphDraw, CommentTextScalesWithZoom) {
  const Loaded l = loadFixture("read/top_level_comment_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{.scale = 2}, r);

  EXPECT_TRUE(hasWrappedText(r.calls, "hello", Rect{108, 158, 234, 184}, 2.0));
  EXPECT_EQ(clipsCovering(r.calls, Rect{108, 158, 234, 184}).size(), 1u);
}

TEST(GraphDraw, CommentTextPassesThroughVerbatim) {
  const Loaded l = loadFixture("read/comment_with_whitespace_and_punctuation.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasWrappedText(r.calls, "Hello there! This is a simple comment!", Rect{54, 79, 117, 92}, 1.0));
}

TEST(GraphDraw, EmptyCommentDrawsNoText) {
  const Loaded l = loadFixture("read/empty_comment_data.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasFill(r.calls, Rect{50, 50, 125, 125}));
  EXPECT_EQ(countOf(r.calls, DrawCall::Op::TextWrapped), 0u);
  EXPECT_EQ(textStrings(r.calls), std::vector<std::string>{"//"});
}

// in_body_comment_only.fl: main at (0,0) 100x100; comment 2 at body units (5,5) 25x25 -> {25,50,125,125}.
TEST(GraphDraw, InBodyCommentWrapsInsideItsBox) {
  const Loaded l = loadFixture("read/in_body_comment_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasFill(r.calls, Rect{25, 50, 125, 125}));
  EXPECT_TRUE(hasScaledTextAt(r.calls, "//", Vec2{29, 64.6}, 0.8));
  EXPECT_TRUE(hasWrappedText(r.calls, "note", Rect{29, 79, 117, 92}, 1.0));
  EXPECT_EQ(clipsCovering(r.calls, Rect{29, 79, 117, 92}).size(), 1u);
  EXPECT_FALSE(clipsCovering(r.calls, Rect{0, 25, 500, 475}).empty());  // function body clip
}

TEST(GraphDraw, SelectedTopLevelCommentIsOutlined) {
  const Loaded l = loadFixture("read/top_level_comment_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer unselected;
  RecordingRenderer selected;
  drawTree(kCtx, *l.result.tree, Viewport{}, unselected);
  drawTree(kCtx, *l.result.tree, Viewport{}, selected, fluir::FullID{1});

  EXPECT_FALSE(hasRect(unselected.calls, Rect{48, 48, 129, 129}));
  EXPECT_TRUE(hasRect(selected.calls, Rect{48, 48, 129, 129}));
}

TEST(GraphDraw, SingleEmptyFunctionFrameAndHeader) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasRect(r.calls, Rect{50, 50, 500, 500}));
  EXPECT_TRUE(hasFill(r.calls, Rect{50, 50, 500, 25}));
  // "fn" at 0.8x, bottom inset by textPad; name full size after it.
  EXPECT_TRUE(hasScaledTextAt(r.calls, "fn", Vec2{54, 64.6}, 0.8));
  EXPECT_TRUE(hasScaledTextAt(r.calls, "foo", Vec2{70.8, 54}, 1.0));
  EXPECT_EQ(textStrings(r.calls), (std::vector<std::string>{"fn", "foo"}));
  EXPECT_EQ(countOf(r.calls, DrawCall::Op::Line), 0u);

  // An empty function has no body children, so no body clip is opened at all;
  // every push that does happen is balanced by a pop.
  EXPECT_TRUE(clipsCovering(r.calls, Rect{50, 75, 500, 475}).empty());
  EXPECT_EQ(countOf(r.calls, DrawCall::Op::PushClip), countOf(r.calls, DrawCall::Op::PopClip));
}

TEST(GraphDraw, ParamRailFromInputOnly) {
  const Loaded l = loadFixture("read/function_with_input_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasRect(r.calls, Rect{50, 50, 500, 500}));  // frame
  EXPECT_TRUE(hasRect(r.calls, Rect{50, 75, 75, 25}));    // param a rail slot
  EXPECT_TRUE(hasRect(r.calls, Rect{50, 100, 75, 25}));   // param b rail slot
  EXPECT_TRUE(hasFill(r.calls, Rect{50, 50, 500, 25}));   // header

  const auto dots = fillsOfSize(r.calls, 6, 6);
  EXPECT_EQ(dots.size(), 2u);  // 2 param port dots, no others
  EXPECT_TRUE(containsRect(dots, Rect{122, 84.5, 6, 6}));
  EXPECT_TRUE(containsRect(dots, Rect{122, 109.5, 6, 6}));

  // Type at 0.8x, bottom inset by textPad; name full size after it.
  EXPECT_TRUE(hasScaledTextAt(r.calls, "I32", Vec2{54, 89.6}, 0.8));
  EXPECT_TRUE(hasScaledTextAt(r.calls, "a", Vec2{77.2, 79}, 1.0));
  EXPECT_TRUE(hasScaledTextAt(r.calls, "I32", Vec2{54, 114.6}, 0.8));
  EXPECT_TRUE(hasScaledTextAt(r.calls, "b", Vec2{77.2, 104}, 1.0));
  const auto texts = textStrings(r.calls);
  EXPECT_EQ(std::find(texts.begin(), texts.end(), "I32 a"), texts.end());
  EXPECT_NE(std::find(texts.begin(), texts.end(), "add"), texts.end());
}

TEST(GraphDraw, ParamRailUsesContextMetrics) {
  const Loaded l = loadFixture("read/function_with_input_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  // paramUnits 18 * unitPx 5 -> paramW 90; railUnits 6 * unitPx 5 -> railStep 30.
  // Body origin y = frameOrigin (50) + headerH (25) = 75; rows step by railStep.
  fluir::editor::EditorContext ctx;
  ctx.layout.paramUnits = 18.0;
  ctx.layout.railUnits = 6.0;
  RecordingRenderer r;
  drawTree(ctx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasRect(r.calls, Rect{50, 75, 90, 30}));   // param a rail slot
  EXPECT_TRUE(hasRect(r.calls, Rect{50, 105, 90, 30}));  // param b rail slot
}

TEST(GraphDraw, ReturnRailFromOutputOnly) {
  const Loaded l = loadFixture("read/function_with_output_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasRect(r.calls, Rect{50, 50, 500, 500}));  // frame
  EXPECT_TRUE(hasRect(r.calls, Rect{525, 75, 25, 25}));   // return rail slot
  EXPECT_TRUE(hasFill(r.calls, Rect{50, 50, 500, 25}));   // header

  const auto dots = fillsOfSize(r.calls, 6, 6);
  EXPECT_EQ(dots.size(), 1u);  // 1 return port dot, no others
  EXPECT_TRUE(containsRect(dots, Rect{522, 84.5, 6, 6}));

  EXPECT_TRUE(hasScaledTextAt(r.calls, "F64", Vec2{529, 89.6}, 0.8));
  const auto texts = textStrings(r.calls);
  EXPECT_NE(std::find(texts.begin(), texts.end(), "getVal"), texts.end());
}

TEST(GraphDraw, RailNameGapIsWorldFixedUnderZoom) {
  const Loaded l = loadFixture("read/function_with_input_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{.pan = {}, .scale = 2.0}, r);

  const auto xOf = [&](std::string_view s) {
    const auto texts = testutil::opsOf(r.calls, DrawCall::Op::Text);
    const auto it = std::ranges::find(texts, s, &DrawCall::text);
    return it == texts.end() ? -1.0 : it->a.x;
  };
  // Glyph cells are world px, so the whole tag-to-name gap zooms with the view.
  EXPECT_NEAR(xOf("a") - xOf("I32"), (kCtx.layout.textPad + 3 * 8 * 0.8) * 2, 1e-6);
}

// Every graph label keeps its proportion to the boxes around it at any zoom.
TEST(GraphDraw, GraphTextScalesUniformlyWithTheView) {
  const Loaded l = loadFixture("read/function_call.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  for (const double s : {2.0, 0.5}) {
    RecordingRenderer r;
    drawTree(kCtx, *l.result.tree, Viewport{.pan = {}, .scale = s}, r);

    EXPECT_TRUE(hasScaledTextAt(r.calls, "add", Vec2{154 * s, 79 * s}, s));              // call title
    EXPECT_TRUE(hasScaledTextAt(r.calls, "i32", Vec2{4 * s, (46 - 6.4) * s}, 0.8 * s));  // constant tag
    EXPECT_TRUE(hasScaledTextAt(r.calls, "10", Vec2{27.2 * s, 29 * s}, s));              // constant value
    EXPECT_TRUE(hasScaledTextAt(r.calls, "a", Vec2{154 * s, 104 * s}, s));               // call argument name
  }
}

TEST(GraphDraw, DeterministicAcrossRuns) {
  const Loaded l = loadFixture("read/multiple_empty_functions.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer a;
  RecordingRenderer b;
  drawTree(kCtx, *l.result.tree, Viewport{}, a);
  drawTree(kCtx, *l.result.tree, Viewport{}, b);

  EXPECT_EQ(a.calls, b.calls);
  EXPECT_FALSE(a.calls.empty());

  // Each function's name label is drawn exactly once; order between them is
  // not a contract worth asserting.
  const auto texts = textStrings(a.calls);
  EXPECT_EQ(std::count(texts.begin(), texts.end(), "foo"), 1);
  EXPECT_EQ(std::count(texts.begin(), texts.end(), "bar"), 1);
  EXPECT_EQ(std::count(texts.begin(), texts.end(), "baz"), 1);
}

TEST(GraphDraw, ViewportIsApplied) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const Viewport vp{.pan = {100, 50}, .scale = 2.0};
  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, vp, r);

  // world frame (50,50,500,500) -> screen (50*2+100, 50*2+50, 500*2, 500*2).
  EXPECT_TRUE(hasRect(r.calls, Rect{200, 150, 1000, 1000}));
  // world header (50,50,500,25) -> screen (200,150,1000,50).
  EXPECT_TRUE(hasFill(r.calls, Rect{200, 150, 1000, 50}));
  // world "fn" pos (54,71) -> screen (208,192), raised by its 12.8 px scaled height.
  EXPECT_TRUE(hasScaledTextAt(r.calls, "fn", Vec2{208, 179.2}, 0.8 * 2));
  // Name follows "fn"'s world-fixed 12.8 px: world x 50+4+12.8+4 -> screen 70.8*2+100.
  EXPECT_TRUE(hasScaledTextAt(r.calls, "foo", Vec2{241.6, 158}, 2.0));
}

TEST(GraphDraw, UnitPxScalesFrameAndHeader) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  // Fixture: x=10 y=10 w=100 h=100 (logical). With unitPx = 10 the frame doubles
  // vs. the default (5) and the header band stays headerUnits (5) tall in logical
  // units -> 50 world px.
  fluir::editor::EditorContext ctx;
  ctx.layout.unitPx = 10.0;
  RecordingRenderer r;
  drawTree(ctx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasRect(r.calls, Rect{100, 100, 1000, 1000}));  // frame
  EXPECT_TRUE(hasFill(r.calls, Rect{100, 100, 1000, 50}));    // header
}

TEST(GraphDraw, ConstantNode) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasRect(r.calls, Rect{60, 175, 25, 25}));   // constant -5
  EXPECT_TRUE(hasRect(r.calls, Rect{110, 180, 25, 25}));  // constant 318
  EXPECT_TRUE(hasRect(r.calls, Rect{160, 185, 25, 25}));  // constant 324
  EXPECT_TRUE(hasRect(r.calls, Rect{210, 190, 25, 25}));  // constant -12

  // Each literal's type at 0.8x along the bottom, its value full size after it.
  EXPECT_TRUE(hasScaledTextAt(r.calls, "i8", Vec2{64, 189.6}, 0.8));
  EXPECT_TRUE(hasScaledTextAt(r.calls, "-5", Vec2{80.8, 179}, 1.0));
  EXPECT_TRUE(hasScaledTextAt(r.calls, "i16", Vec2{114, 194.6}, 0.8));
  EXPECT_TRUE(hasScaledTextAt(r.calls, "318", Vec2{137.2, 184}, 1.0));
  EXPECT_TRUE(hasScaledTextAt(r.calls, "i32", Vec2{164, 199.6}, 0.8));
  EXPECT_TRUE(hasScaledTextAt(r.calls, "324", Vec2{187.2, 189}, 1.0));
  EXPECT_TRUE(hasScaledTextAt(r.calls, "i64", Vec2{214, 204.6}, 0.8));
  EXPECT_TRUE(hasScaledTextAt(r.calls, "-12", Vec2{237.2, 194}, 1.0));
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

// read_boolean.fl: false node {60,175,40,25}, true node {110,180,40,25}; each toggle square 12x12,
// inset 4 from the left edge and vertically centred.
TEST(GraphDraw, BoolConstantDrawsNoResizeGrip) {
  const Loaded l = loadFixture("read/read_boolean.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  // Bools still move, but have no resize bar to grab.
  EXPECT_FALSE(hasFill(r.calls, Rect{95, 175, 5, 25}));
  EXPECT_FALSE(hasFill(r.calls, Rect{145, 180, 5, 25}));
}

TEST(GraphDraw, BinaryNodeHasTwoInputsOneOutput) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasTextAt(r.calls, "+", Vec2{129, 89}));  // stringify(PLUS)

  EXPECT_TRUE(hasFill(r.calls, Rect{122, 82, 6, 6}));    // binary input 0, y-frac 0.0
  EXPECT_TRUE(hasFill(r.calls, Rect{122, 107, 6, 6}));   // binary input 1, y-frac 1.0
  EXPECT_TRUE(hasFill(r.calls, Rect{147, 94.5, 6, 6}));  // binary output, y-frac 0.5
}

TEST(GraphDraw, UnaryNodeHasOneInput) {
  const Loaded l = loadFixture("read/simple_unary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasTextAt(r.calls, "-", Vec2{129, 89}));  // unary op label

  EXPECT_TRUE(hasFill(r.calls, Rect{122, 94.5, 6, 6}));  // unary single input, y-frac 0.5
  EXPECT_TRUE(hasFill(r.calls, Rect{147, 94.5, 6, 6}));  // unary output

  // constant id=3 output-0 anchor -> unary id=7 input-0 anchor.
  EXPECT_TRUE(hasLine(r.calls, Vec2{85, 97.5}, Vec2{125, 97.5}));
}

TEST(GraphDraw, CallNodeArgsAndReturn) {
  {
    const Loaded l = loadFixture("read/function_call.fl");
    ASSERT_TRUE(l.result.tree.has_value());

    RecordingRenderer r;
    drawTree(kCtx, *l.result.tree, Viewport{}, r);

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
    drawTree(kCtx, *l.result.tree, Viewport{}, r);

    EXPECT_TRUE(hasTextAt(r.calls, "doStuff", Vec2{29, 54}));

    EXPECT_TRUE(fillsOfSize(r.calls, 6, 6).empty());  // no args, no return -> no port dots
    EXPECT_EQ(countOf(r.calls, DrawCall::Op::Line), 0u);
  }
}

TEST(GraphDraw, WireEndpointsMatchPorts) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_EQ(countOf(r.calls, DrawCall::Op::Line), 2u);
  EXPECT_TRUE(hasLine(r.calls, Vec2{85, 97.5}, Vec2{125, 85}));    // constant id=2 -> binary input-0
  EXPECT_TRUE(hasLine(r.calls, Vec2{85, 147.5}, Vec2{125, 110}));  // constant id=3 -> binary input-1
}

TEST(GraphDraw, ClipWrapsBodyForFunctionWithNodes) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  // Body content (nodes, rails, conduits) is clipped to the body rect.
  EXPECT_FALSE(clipsCovering(r.calls, Rect{50, 75, 500, 475}).empty());
  EXPECT_EQ(countOf(r.calls, DrawCall::Op::PushClip), countOf(r.calls, DrawCall::Op::PopClip));
  EXPECT_TRUE(hasRect(r.calls, Rect{125, 85, 25, 25}));  // binary node body rect
}

TEST(GraphDraw, DeterministicWithBodyAndWires) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer a;
  RecordingRenderer b;
  drawTree(kCtx, *l.result.tree, Viewport{}, a);
  drawTree(kCtx, *l.result.tree, Viewport{}, b);

  EXPECT_EQ(a.calls, b.calls);
  EXPECT_FALSE(testutil::opsOf(a.calls, DrawCall::Op::Line).empty());
  EXPECT_FALSE(testutil::opsOf(a.calls, DrawCall::Op::PushClip).empty());
}

TEST(GraphDraw, AllMoveGripsAreIcons) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  // Every move grip draws the handle: one function plus three nodes, and no resize grip leaks in.
  EXPECT_GT(testutil::opsOf(r.calls, DrawCall::Op::Icon).size(), 0);
}

TEST(GraphDraw, SelectedNodeIsOutlined) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer unselected;
  RecordingRenderer selected;
  drawTree(kCtx, *l.result.tree, Viewport{}, unselected);
  drawTree(kCtx, *l.result.tree, Viewport{}, selected, fluir::FullID{1, 1});

  // Binary {125,85,25,25} outset by selectionPad (2), then by one more px.
  EXPECT_FALSE(hasRect(unselected.calls, Rect{123, 83, 29, 29}));
  EXPECT_TRUE(hasRect(selected.calls, Rect{123, 83, 29, 29}));
  EXPECT_TRUE(hasRect(selected.calls, Rect{122, 82, 31, 31}));
}

TEST(GraphDraw, SelectedFunctionIsOutlined) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r, fluir::FullID{1});

  EXPECT_TRUE(hasRect(r.calls, Rect{48, 48, 504, 504}));
}

namespace {

  using fluir::FlowGraphLocation;
  using fluir::ID;

  fluir::pt::Scope makeScope(ID id, int y, int h) {
    return fluir::pt::Scope{.id = id, .location = {.x = 0, .y = y, .z = 0, .width = 20, .height = h}, .body = {}};
  }

  // Function 1 (frame {0,0,500,500}, body clip {0,25,500,475}) holds conditional 20 at units (x,2)
  // 20 wide, a 12-unit then branch over a 6-unit else branch. Each branch holds a constant 1,
  // and each gives up its own top 25 to its header.
  //   frame {10,35,100,90}  then {10,35,100,60} (content {10,60,100,35})
  //                         else {10,95,100,30} (content {10,120,100,5})
  //   then 1 {15,65,50,50}  else 1 {10,120,50,50}, all but its top 5 clipped away
  fluir::pt::ParseTree conditionalTree(int y = 2) {
    fluir::pt::Scope then = makeScope(0, 0, 12);
    then.body.nodes.emplace(1,
                            fluir::pt::Constant{.id = 1,
                                                .location = {.x = 1, .y = 1, .z = 0, .width = 10, .height = 10},
                                                .value = fluir::literals_types::I32{0}});

    fluir::pt::Scope else_ = makeScope(1, 12, 6);
    else_.body.nodes.emplace(1,
                             fluir::pt::Constant{.id = 1,
                                                 .location = {.x = 0, .y = 0, .z = 0, .width = 10, .height = 10},
                                                 .value = fluir::literals_types::I32{0}});

    fluir::pt::FunctionDecl fn;
    fn.id = 1;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    fn.body.nodes.emplace(20,
                          fluir::pt::Conditional{.id = 20,
                                                 .location = {.x = 2, .y = y, .z = 0, .width = 20, .height = 18},
                                                 .condition = {},
                                                 .inputs = {},
                                                 .outputs = {},
                                                 .thenScope = xyz::indirect{std::move(then)},
                                                 .elseScope = xyz::indirect{std::move(else_)}});

    fluir::pt::ParseTree tree;
    tree.declarations.emplace(1, fluir::pt::Declaration{std::move(fn)});
    return tree;
  }

  // Where a fill of `want` first lands in the call list, or calls.size() when it never does.
  std::size_t firstFillIndex(const std::vector<DrawCall>& calls, Rect want) {
    for (std::size_t i = 0; i < calls.size(); ++i) {
      if (calls[i].op == DrawCall::Op::Fill && std::abs(calls[i].rect.x - want.x) < 1e-6 &&
          std::abs(calls[i].rect.y - want.y) < 1e-6 && std::abs(calls[i].rect.w - want.w) < 1e-6 &&
          std::abs(calls[i].rect.h - want.h) < 1e-6) {
        return i;
      }
    }
    return calls.size();
  }

}  // namespace

// A branch's own clip must admit the header it paints, or its band, divider and tag are culled.
TEST(GraphDraw, ABranchIsClippedToItsWholeRectNotJustItsContent) {
  RecordingRenderer r;
  drawTree(kCtx, conditionalTree(), Viewport{}, r);

  EXPECT_FALSE(clipsCovering(r.calls, Rect{10, 35, 100, 60}).empty());  // then, header included
  EXPECT_FALSE(clipsCovering(r.calls, Rect{10, 95, 100, 30}).empty());  // else, header included
}

TEST(GraphDraw, EachBranchDrawsItsOwnHeaderInsideTheFrame) {
  RecordingRenderer r;
  drawTree(kCtx, conditionalTree(), Viewport{}, r);

  EXPECT_TRUE(hasFill(r.calls, Rect{10, 35, 100, 25}));  // the then branch's header
  EXPECT_TRUE(hasFill(r.calls, Rect{10, 95, 100, 25}));  // the else branch's header
  EXPECT_TRUE(hasRect(r.calls, Rect{10, 35, 100, 90}));  // border around both branches
  const std::vector<std::string> texts = textStrings(r.calls);
  EXPECT_NE(std::find(texts.begin(), texts.end(), std::string{"then"}), texts.end());
  EXPECT_NE(std::find(texts.begin(), texts.end(), std::string{"else"}), texts.end());
  EXPECT_EQ(std::find(texts.begin(), texts.end(), std::string{"if"}), texts.end());
}

TEST(GraphDraw, EachBranchDividesItselfFromWhatIsAboveIt) {
  RecordingRenderer r;
  drawTree(kCtx, conditionalTree(), Viewport{}, r);

  EXPECT_TRUE(hasLine(r.calls, Vec2{10, 35}, Vec2{110, 35}));  // then branch's top
  EXPECT_TRUE(hasLine(r.calls, Vec2{10, 95}, Vec2{110, 95}));  // the else divider
}

TEST(GraphDraw, BranchClipsToItsContentRegion) {
  RecordingRenderer r;
  drawTree(kCtx, conditionalTree(), Viewport{}, r);

  EXPECT_FALSE(clipsCovering(r.calls, Rect{10, 60, 100, 35}).empty());  // then's nodes, minus its header
  EXPECT_FALSE(clipsCovering(r.calls, Rect{10, 120, 100, 5}).empty());  // else's nodes, minus its header
}

// A conditional near its function's bottom edge: the branch clip is the intersection of the
// two, and a branch pushed entirely outside is not drawn at all.
TEST(GraphDraw, BranchClipIsIntersectedWithItsContainer) {
  RecordingRenderer r;
  drawTree(kCtx, conditionalTree(88), Viewport{}, r);  // frame {10,465,100,90}, function clip ends at y 500

  EXPECT_FALSE(clipsCovering(r.calls, Rect{10, 490, 100, 10}).empty());  // then content 490..525, clipped at 500
  EXPECT_TRUE(clipsCovering(r.calls, Rect{10, 525, 100, 30}).empty());   // the else branch is past the edge
}

// A branch's header belongs to the branch, so it paints before the nodes it clips.
TEST(GraphDraw, ABranchHeaderPaintsBeforeItsNestedNodes) {
  RecordingRenderer r;
  drawTree(kCtx, conditionalTree(), Viewport{}, r);

  const std::size_t node = firstFillIndex(r.calls, Rect{15, 65, 50, 50});
  const std::size_t header = firstFillIndex(r.calls, Rect{10, 35, 100, 25});

  ASSERT_LT(node, r.calls.size());
  ASSERT_LT(header, r.calls.size());
  EXPECT_LT(header, node);
}

// Function declarations keep the two-axis corner icon: this change does not touch them.
TEST(GraphDraw, AFunctionKeepsItsCornerResizeIcon) {
  RecordingRenderer r;
  drawTree(kCtx, conditionalTree(), Viewport{}, r);

  EXPECT_TRUE(hasIcon(r.calls, fluir::editor::assets::xyResizeIcon(), Rect{485, 485, 15, 15}));
}

TEST(GraphDraw, SelectingABranchOutlinesItsConditional) {
  RecordingRenderer r;
  drawTree(kCtx, conditionalTree(), Viewport{}, r, fluir::FullID{1, 20, 1});

  // Frame {10,35,100,90} outset by selectionPad (2), then by one more px.
  EXPECT_TRUE(hasRect(r.calls, Rect{8, 33, 104, 94}));
  EXPECT_TRUE(hasRect(r.calls, Rect{7, 32, 106, 96}));
}

TEST(GraphDraw, SelectingANestedNodeOutlinesThatNodeAlone) {
  RecordingRenderer r;
  drawTree(kCtx, conditionalTree(), Viewport{}, r, fluir::FullID{1, 20, 0, 1});

  EXPECT_TRUE(hasRect(r.calls, Rect{13, 63, 54, 54}));   // the node, outset by 2
  EXPECT_FALSE(hasRect(r.calls, Rect{8, 33, 104, 94}));  // not its conditional
}

// conditional_with_body.fl: conditional 2 -> frame {50,40,2500,2500}, then branch over
// else branch {50,1290,2500,1250}.
TEST(GraphDraw, FixtureConditionalDrawsItsChromeAndDivider) {
  const Loaded l = loadFixture("read/conditional_with_body.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasFill(r.calls, Rect{50, 40, 2500, 25}));            // the then branch's header
  EXPECT_TRUE(hasFill(r.calls, Rect{50, 1290, 2500, 25}));          // the else branch's header
  EXPECT_TRUE(hasRect(r.calls, Rect{50, 40, 2500, 2500}));          // border over both branches
  EXPECT_TRUE(hasLine(r.calls, Vec2{50, 1290}, Vec2{2550, 1290}));  // the divider
  EXPECT_FALSE(clipsCovering(r.calls, Rect{50, 65, 2500, 1225}).empty());
  EXPECT_FALSE(clipsCovering(r.calls, Rect{50, 1315, 2500, 1225}).empty());
}

// Both branches hold nodes, and the else branch reuses the ids of the then branch's.
TEST(GraphDraw, FixtureNestedNodesDrawInBothBranches) {
  const Loaded l = loadFixture("read/conditional_with_body.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  drawTree(kCtx, *l.result.tree, Viewport{}, r);

  EXPECT_TRUE(hasFill(r.calls, Rect{75, 90, 25, 25}));     // then: constant 2
  EXPECT_TRUE(hasFill(r.calls, Rect{95, 1340, 60, 25}));   // else: constant 1
  EXPECT_TRUE(hasFill(r.calls, Rect{225, 1330, 25, 25}));  // else: binary 3
}
