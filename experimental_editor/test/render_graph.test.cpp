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

// read/top_level_comment_only.fl has no <function>, so no frame is emitted.
TEST(RenderGraph, EmptyOfFunctionsEmitsNoFrames) {
  const Loaded l = loadFixture("read/top_level_comment_only.fl");
  ASSERT_TRUE(l.result.tree.has_value());

  RecordingRenderer r;
  fluir::editor::renderGraph(*l.result.tree, Viewport{}, r);

  EXPECT_TRUE(opsOf(r.calls, DrawCall::Op::Rect).empty());
  EXPECT_TRUE(r.calls.empty());
}

// read/single_empty_function.fl: <function name="foo" x=10 y=10 z=3 w=100 h=100>.
// world origin = (50,50); frame = (50,50,500,500); header = (50,50,500,25);
// name text at (54,54).
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

  // Scope for P1c-a: no wires, no clip rects.
  EXPECT_TRUE(opsOf(r.calls, DrawCall::Op::Line).empty());
  EXPECT_TRUE(opsOf(r.calls, DrawCall::Op::PushClip).empty());
  EXPECT_TRUE(opsOf(r.calls, DrawCall::Op::PopClip).empty());
}

// read/function_with_input_only.fl: <function name="add" x=10 y=10 z=3 w=100 h=100>
// with params a,b (I32). Param 0 rect = (50,75,75,25); right-edge output dot
// centred on (125,87.5) => fill rect (122,84.5,6,6). Param text "I32 a" at (54,79).
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

// read/function_with_output_only.fl: <function name="getVal" x=10 y=10 z=3 w=100
// h=100> with a single F64 return. Return rect = (525,75,25,25); left-edge input
// dot centred on (525,87.5) => fill rect (522,84.5,6,6). Return text "F64" at (529,79).
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

// read/multiple_empty_functions.fl: three functions (ids 1,7,2, all z=3). Two
// renderGraph runs on the same tree must produce identical call sequences.
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

// A non-identity Viewport maps world -> screen before every Renderer call:
// top-left * scale + pan, size * scale.
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
