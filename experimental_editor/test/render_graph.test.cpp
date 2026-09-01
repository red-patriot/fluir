#include "../include/editor/core/render_graph.hpp"

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../include/editor/core/collecting_sink.hpp"
#include "../include/editor/core/loader.hpp"
#include "compiler/utility/context.hpp"
#include "recording_renderer.hpp"

namespace {

  namespace fs = std::filesystem;

  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::DrawCall;
  using testutil::RecordingRenderer;

  // Body coordinates are frame-origin + kHeaderH (25 world px): body content is
  // rendered below the header band, so every body-relative y is offset by +25.
  //
  // Same fixture-loading shape as loader.test.cpp: each load owns its Context +
  // CollectingSink, version checks off (fixtures declare <version>0.1.3</version>).
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

  std::vector<DrawCall> opsOf(const std::vector<DrawCall>& calls, DrawCall::Op op) {
    std::vector<DrawCall> out;
    for (const auto& c : calls) {
      if (c.op == op) {
        out.push_back(c);
      }
    }
    return out;
  }

  void expectRectNear(const Rect& got, const Rect& want, double tol = 1e-6) {
    EXPECT_NEAR(got.x, want.x, tol);
    EXPECT_NEAR(got.y, want.y, tol);
    EXPECT_NEAR(got.w, want.w, tol);
    EXPECT_NEAR(got.h, want.h, tol);
  }

  void expectVecNear(const Vec2& got, const Vec2& want, double tol = 1e-6) {
    EXPECT_NEAR(got.x, want.x, tol);
    EXPECT_NEAR(got.y, want.y, tol);
  }

}  // namespace

TEST(RenderGraph, EmptyOfFunctionsEmitsNoFrames) {
  const Loaded l = loadFixture("read/top_level_comment_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(*l.result.tree, Viewport{}, r);

  EXPECT_TRUE(opsOf(r.calls, DrawCall::Op::Rect).empty());
  EXPECT_TRUE(r.calls.empty());
}

TEST(RenderGraph, SingleEmptyFunctionFrameAndHeader) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(*l.result.tree, Viewport{}, r);

  ASSERT_FALSE(r.calls.empty());
  EXPECT_EQ(r.calls.front().op, DrawCall::Op::Rect);
  expectRectNear(r.calls.front().rect, Rect{50, 50, 500, 500});

  const auto fills = opsOf(r.calls, DrawCall::Op::Fill);
  ASSERT_FALSE(fills.empty());
  expectRectNear(fills.front().rect, Rect{50, 50, 500, 25});

  const auto texts = opsOf(r.calls, DrawCall::Op::Text);
  ASSERT_EQ(texts.size(), 1u);
  EXPECT_EQ(texts.front().text, "foo");
  expectVecNear(texts.front().a, Vec2{54, 54});

  // The body Subview is constructed before the empty-body early-return (ports and
  // rails draw on it), so an empty function pushes exactly one clip for the body
  // rect and pops it once at scope exit.
  const auto pushes = opsOf(r.calls, DrawCall::Op::PushClip);
  const auto pops = opsOf(r.calls, DrawCall::Op::PopClip);
  ASSERT_EQ(pushes.size(), 1u);
  ASSERT_EQ(pops.size(), 1u);
  // toScreen(Rect{0,0,w,h}) from bodyOrigin = frameOrigin + kHeaderH (25 world px).
  expectRectNear(pushes.front().rect, Rect{50, 75, 500, 500});
  EXPECT_TRUE(opsOf(r.calls, DrawCall::Op::Line).empty());
}

TEST(RenderGraph, ParamRailFromInputOnly) {
  const Loaded l = loadFixture("read/function_with_input_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(*l.result.tree, Viewport{}, r);

  const auto rects = opsOf(r.calls, DrawCall::Op::Rect);
  ASSERT_EQ(rects.size(), 3u);  // frame + 2 param rects
  expectRectNear(rects[0].rect, Rect{50, 50, 500, 500});
  expectRectNear(rects[1].rect, Rect{50, 75, 75, 25});
  expectRectNear(rects[2].rect, Rect{50, 100, 75, 25});

  const auto fills = opsOf(r.calls, DrawCall::Op::Fill);
  ASSERT_EQ(fills.size(), 3u);  // header + 2 port dots
  expectRectNear(fills[0].rect, Rect{50, 50, 500, 25});
  expectRectNear(fills[1].rect, Rect{122, 84.5, 6, 6});
  expectRectNear(fills[2].rect, Rect{122, 109.5, 6, 6});

  const auto texts = opsOf(r.calls, DrawCall::Op::Text);
  ASSERT_EQ(texts.size(), 3u);  // name + 2 param labels
  EXPECT_EQ(texts[0].text, "add");
  EXPECT_EQ(texts[1].text, "I32 a");
  expectVecNear(texts[1].a, Vec2{54, 79});
  EXPECT_EQ(texts[2].text, "I32 b");
  expectVecNear(texts[2].a, Vec2{54, 104});
}

TEST(RenderGraph, ReturnRailFromOutputOnly) {
  const Loaded l = loadFixture("read/function_with_output_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(*l.result.tree, Viewport{}, r);

  const auto rects = opsOf(r.calls, DrawCall::Op::Rect);
  ASSERT_EQ(rects.size(), 2u);  // frame + return rect
  expectRectNear(rects[0].rect, Rect{50, 50, 500, 500});
  expectRectNear(rects[1].rect, Rect{525, 75, 25, 25});

  const auto fills = opsOf(r.calls, DrawCall::Op::Fill);
  ASSERT_EQ(fills.size(), 2u);  // header + input port dot
  expectRectNear(fills[0].rect, Rect{50, 50, 500, 25});
  expectRectNear(fills[1].rect, Rect{522, 84.5, 6, 6});

  const auto texts = opsOf(r.calls, DrawCall::Op::Text);
  ASSERT_EQ(texts.size(), 2u);  // name + return label
  EXPECT_EQ(texts[0].text, "getVal");
  EXPECT_EQ(texts[1].text, "F64");
  expectVecNear(texts[1].a, Vec2{529, 79});
}

TEST(RenderGraph, DeterministicAcrossRuns) {
  const Loaded l = loadFixture("read/multiple_empty_functions.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer a;
  RecordingRenderer b;
  fluir::editor::renderGraph(*l.result.tree, Viewport{}, a);
  fluir::editor::renderGraph(*l.result.tree, Viewport{}, b);

  EXPECT_EQ(a.calls, b.calls);
  EXPECT_FALSE(a.calls.empty());

  // Sorted by (z, id) => foo(1), bar(2), baz(7).
  const auto texts = opsOf(a.calls, DrawCall::Op::Text);
  ASSERT_EQ(texts.size(), 3u);
  EXPECT_EQ(texts[0].text, "foo");
  EXPECT_EQ(texts[1].text, "bar");
  EXPECT_EQ(texts[2].text, "baz");
}

TEST(RenderGraph, ViewportIsApplied) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  const Viewport vp{.pan = {100, 50}, .scale = 2.0};
  RecordingRenderer r;
  fluir::editor::renderGraph(*l.result.tree, vp, r);

  ASSERT_FALSE(r.calls.empty());
  EXPECT_EQ(r.calls.front().op, DrawCall::Op::Rect);
  // world frame (50,50,500,500) -> screen (50*2+100, 50*2+50, 500*2, 500*2).
  expectRectNear(r.calls.front().rect, Rect{200, 150, 1000, 1000});

  const auto fills = opsOf(r.calls, DrawCall::Op::Fill);
  ASSERT_FALSE(fills.empty());
  // world header (50,50,500,25) -> screen (200,150,1000,50).
  expectRectNear(fills.front().rect, Rect{200, 150, 1000, 50});

  const auto texts = opsOf(r.calls, DrawCall::Op::Text);
  ASSERT_EQ(texts.size(), 1u);
  // world text pos (54,54) -> screen (54*2+100, 54*2+50).
  expectVecNear(texts.front().a, Vec2{208, 158});
}

TEST(RenderGraph, ConstantNode) {
  const Loaded l = loadFixture("read/int_constants.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(*l.result.tree, Viewport{}, r);

  const auto rects = opsOf(r.calls, DrawCall::Op::Rect);
  ASSERT_EQ(rects.size(), 5u);  // frame + 4 constants
  expectRectNear(rects[1].rect, Rect{60, 175, 25, 25});

  const auto texts = opsOf(r.calls, DrawCall::Op::Text);
  ASSERT_EQ(texts.size(), 5u);  // name + 4 literals
  EXPECT_EQ(texts[1].text, "-5");
  expectVecNear(texts[1].a, Vec2{64, 179});
  EXPECT_EQ(texts[2].text, "318");
  EXPECT_EQ(texts[3].text, "324");
  EXPECT_EQ(texts[4].text, "-12");

  const auto fills = opsOf(r.calls, DrawCall::Op::Fill);
  ASSERT_EQ(fills.size(), 5u);  // header + 4 output dots, no input dots
  expectRectNear(fills[0].rect, Rect{50, 50, 500, 25});
  expectRectNear(fills[1].rect, Rect{82, 184.5, 6, 6});

  EXPECT_TRUE(opsOf(r.calls, DrawCall::Op::Line).empty());
}

TEST(RenderGraph, BinaryNodeHasTwoInputsOneOutput) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(*l.result.tree, Viewport{}, r);

  const auto texts = opsOf(r.calls, DrawCall::Op::Text);
  ASSERT_GE(texts.size(), 2u);
  EXPECT_EQ(texts[1].text, "+");  // stringify(PLUS); binary sorts first (id 1)
  expectVecNear(texts[1].a, Vec2{129, 89});

  const auto fills = opsOf(r.calls, DrawCall::Op::Fill);
  ASSERT_EQ(fills.size(), 6u);                           // header + (2 in + 1 out) + const out + const out
  expectRectNear(fills[1].rect, Rect{122, 82, 6, 6});    // binary input 0, y-frac 0.0
  expectRectNear(fills[2].rect, Rect{122, 107, 6, 6});   // binary input 1, y-frac 1.0
  expectRectNear(fills[3].rect, Rect{147, 94.5, 6, 6});  // binary output, y-frac 0.5
}

TEST(RenderGraph, UnaryNodeHasOneInput) {
  const Loaded l = loadFixture("read/simple_unary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(*l.result.tree, Viewport{}, r);

  const auto texts = opsOf(r.calls, DrawCall::Op::Text);
  ASSERT_EQ(texts.size(), 3u);    // name + constant + unary op
  EXPECT_EQ(texts[2].text, "-");  // unary sorts after constant (id 7 vs 3)
  expectVecNear(texts[2].a, Vec2{129, 89});

  const auto fills = opsOf(r.calls, DrawCall::Op::Fill);
  ASSERT_EQ(fills.size(), 4u);                           // header + const out + unary in + unary out
  expectRectNear(fills[2].rect, Rect{122, 94.5, 6, 6});  // unary single input, y-frac 0.5
  expectRectNear(fills[3].rect, Rect{147, 94.5, 6, 6});  // unary output

  ASSERT_EQ(opsOf(r.calls, DrawCall::Op::Line).size(), 1u);
}

TEST(RenderGraph, CallNodeArgsAndReturn) {
  {
    const Loaded l = loadFixture("read/function_call.fl");
    ASSERT_TRUE(l.result.tree.has_value());

    RecordingRenderer r;
    fluir::editor::renderGraph(*l.result.tree, Viewport{}, r);

    const auto texts = opsOf(r.calls, DrawCall::Op::Text);
    ASSERT_EQ(texts.size(), 6u);  // main, 10, 20, add, a, b
    EXPECT_EQ(texts[3].text, "add");
    expectVecNear(texts[3].a, Vec2{154, 79});
    EXPECT_EQ(texts[4].text, "a");
    expectVecNear(texts[4].a, Vec2{154, 104});
    EXPECT_EQ(texts[5].text, "b");
    expectVecNear(texts[5].a, Vec2{154, 129});

    const auto fills = opsOf(r.calls, DrawCall::Op::Fill);
    ASSERT_EQ(fills.size(), 6u);  // header + 2 const outs + 2 call arg ins + 1 call return out
    expectRectNear(fills[3].rect, Rect{147, 109.5, 6, 6});  // call arg row 0 input
    expectRectNear(fills[4].rect, Rect{147, 134.5, 6, 6});  // call arg row 1 input
    expectRectNear(fills[5].rect, Rect{207, 102, 6, 6});    // call return output

    ASSERT_EQ(opsOf(r.calls, DrawCall::Op::Line).size(), 1u);  // conduit id=5 index 2 guarded out
  }
  {
    const Loaded l = loadFixture("read/function_call_no_args_no_returns.fl");
    ASSERT_TRUE(l.result.tree.has_value());

    RecordingRenderer r;
    fluir::editor::renderGraph(*l.result.tree, Viewport{}, r);

    const auto texts = opsOf(r.calls, DrawCall::Op::Text);
    ASSERT_EQ(texts.size(), 2u);  // name + call target only
    EXPECT_EQ(texts[1].text, "doStuff");
    expectVecNear(texts[1].a, Vec2{29, 54});

    const auto fills = opsOf(r.calls, DrawCall::Op::Fill);
    ASSERT_EQ(fills.size(), 1u);  // header only; no args, no return

    EXPECT_TRUE(opsOf(r.calls, DrawCall::Op::Line).empty());
  }
}

TEST(RenderGraph, WireEndpointsMatchPorts) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(*l.result.tree, Viewport{}, r);

  const auto lines = opsOf(r.calls, DrawCall::Op::Line);
  ASSERT_EQ(lines.size(), 2u);
  expectVecNear(lines[0].a, Vec2{85, 97.5});   // constant id=2 output-0 anchor
  expectVecNear(lines[0].b, Vec2{125, 85});    // binary id=1 input-0 anchor
  expectVecNear(lines[1].a, Vec2{85, 147.5});  // constant id=3 output-0 anchor
  expectVecNear(lines[1].b, Vec2{125, 110});   // binary id=1 input-1 anchor
}

TEST(RenderGraph, ClipWrapsBodyForFunctionWithNodes) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(*l.result.tree, Viewport{}, r);
  const auto& calls = r.calls;

  ASSERT_EQ(opsOf(calls, DrawCall::Op::PushClip).size(), 1u);
  ASSERT_EQ(opsOf(calls, DrawCall::Op::PopClip).size(), 1u);

  std::size_t pushIdx = calls.size();
  std::size_t popIdx = calls.size();
  for (std::size_t i = 0; i < calls.size(); ++i) {
    if (calls[i].op == DrawCall::Op::PushClip) {
      pushIdx = i;
    }
    if (calls[i].op == DrawCall::Op::PopClip) {
      popIdx = i;
    }
  }
  ASSERT_LT(pushIdx, calls.size());
  ASSERT_LT(popIdx, calls.size());
  expectRectNear(calls[pushIdx].rect, Rect{50, 75, 500, 500});

  std::size_t firstBodyRect = calls.size();
  for (std::size_t i = pushIdx + 1; i < calls.size(); ++i) {
    if (calls[i].op == DrawCall::Op::Rect) {
      firstBodyRect = i;
      break;
    }
  }
  ASSERT_LT(firstBodyRect, calls.size());
  EXPECT_LT(pushIdx, firstBodyRect);
  expectRectNear(calls[firstBodyRect].rect, Rect{125, 85, 25, 25});

  std::size_t lastLine = calls.size();
  for (std::size_t i = 0; i < popIdx; ++i) {
    if (calls[i].op == DrawCall::Op::Line) {
      lastLine = i;
    }
  }
  ASSERT_LT(lastLine, calls.size());
  EXPECT_LT(lastLine, popIdx);
}

TEST(RenderGraph, EmptyFunctionPushesSingleBodyClip) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(*l.result.tree, Viewport{}, r);

  // Empty body: the body Subview still constructs (rails/ports render on it)
  // before the early-return, so exactly one clip push + one matching pop, for
  // the body rect. Same body rect the non-empty ClipWrapsBodyForFunctionWithNodes
  // asserts: toScreen(Rect{0,0,w,h}) at bodyOrigin = frameOrigin + kHeaderH.
  const auto pushes = opsOf(r.calls, DrawCall::Op::PushClip);
  const auto pops = opsOf(r.calls, DrawCall::Op::PopClip);
  ASSERT_EQ(pushes.size(), 1u);
  ASSERT_EQ(pops.size(), 1u);
  expectRectNear(pushes.front().rect, Rect{50, 75, 500, 500});
}

TEST(RenderGraph, DeterministicWithBodyAndWires) {
  const Loaded l = loadFixture("read/simple_binary_expr.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer a;
  RecordingRenderer b;
  fluir::editor::renderGraph(*l.result.tree, Viewport{}, a);
  fluir::editor::renderGraph(*l.result.tree, Viewport{}, b);

  EXPECT_EQ(a.calls, b.calls);
  EXPECT_FALSE(opsOf(a.calls, DrawCall::Op::Line).empty());
  EXPECT_FALSE(opsOf(a.calls, DrawCall::Op::PushClip).empty());
}

TEST(GraphBounds, EmptyTreeIsZero) {
  const Loaded l = loadFixture("read/top_level_comment_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  expectRectNear(fluir::editor::graphBounds(*l.result.tree), Rect{0, 0, 0, 0});
}

TEST(GraphBounds, SingleFunctionIsItsFrame) {
  const Loaded l = loadFixture("read/single_empty_function.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  // Fixture: x=10 y=10 w=100 h=100 (logical); world px = logical * UNIT_PX (5).
  // Same frame rect asserted by SingleEmptyFunctionFrameAndHeader above.
  expectRectNear(fluir::editor::graphBounds(*l.result.tree), Rect{50, 50, 500, 500});
}

TEST(GraphBounds, MultipleFunctionsUnion) {
  const Loaded l = loadFixture("read/multiple_empty_functions.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  // Fixture frames (logical -> world px, * UNIT_PX == 5):
  //   foo x=10  y=10 w=100 h=100 -> world [ 50, 50 ..  550, 550]
  //   baz x=330 y=10 w=100 h=100 -> world [1650, 50 .. 2150, 550]
  //   bar x=210 y=10 w=50  h=70  -> world [1050, 50 .. 1300, 400]
  // union: min (50, 50), max (2150, 550) -> {50, 50, 2100, 500}.
  expectRectNear(fluir::editor::graphBounds(*l.result.tree), Rect{50, 50, 2100, 500});
}
