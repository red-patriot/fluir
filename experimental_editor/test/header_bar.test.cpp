#include "editor/pages/header_bar.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/components/toolbar_actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/module_editor.hpp"
#include "editor/core/viewport.hpp"
#include "recording_renderer.hpp"

// The bar is a ToolbarActor: buttons size themselves from their labels and are
// rowed by `resize`. Contracts here are placement and dispatch, not pixel counts.

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::HeaderBar;
  using fluir::editor::ModuleEditor;
  using fluir::editor::Rect;
  using fluir::editor::Subview;
  using fluir::editor::ToolbarActor;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::hasFill;
  using testutil::hasTextAt;
  using testutil::RecordingRenderer;
  using testutil::textStrings;

  constexpr double kOutputWidth = 800;

  std::vector<testutil::DrawCall> record(RecordingRenderer& renderer, HeaderBar& header, const EditorContext& ctx) {
    const Viewport identity;
    {
      const Subview view{identity, Rect{0, 0, 800, 600}, renderer};
      header.draw(view, ctx);
    }
    return renderer.calls;
  }

  bool hasText(const std::vector<testutil::DrawCall>& calls, const std::string& want) {
    const auto texts = textStrings(calls);
    return std::find(texts.begin(), texts.end(), want) != texts.end();
  }

}  // namespace

TEST(HeaderBar, DrawShowsBarFillAndProgramName) {
  EditorContext ctx;
  RecordingRenderer renderer;

  ModuleEditor editor;

  HeaderBar header(renderer, [] {}, [] {}, [] {}, editor);
  header.setLabel("example.fl");
  header.resize(ctx, kOutputWidth);

  const auto calls = record(renderer, header, ctx);

  // The filename sits to the right of the last left-aligned button.
  const double textX = header.redoButton().bounds().x + header.redoButton().bounds().w + ctx.layout.textPad;
  EXPECT_TRUE(hasFill(calls, Rect{0, 0, kOutputWidth, ctx.layout.chromeHeaderPx}));
  EXPECT_TRUE(hasTextAt(calls, "example.fl", Vec2{textX, ctx.layout.textPad}));
}

TEST(HeaderBar, DrawSkipsNameWhenNoLabelIsSet) {
  EditorContext ctx;
  RecordingRenderer renderer;

  ModuleEditor editor;

  HeaderBar header(renderer, [] {}, [] {}, [] {}, editor);
  header.resize(ctx, kOutputWidth);

  const auto calls = record(renderer, header, ctx);

  EXPECT_TRUE(hasFill(calls, Rect{0, 0, kOutputWidth, ctx.layout.chromeHeaderPx}));
  // Only the buttons' own labels are drawn -- no filename.
  EXPECT_TRUE(hasText(calls, "Save"));
  EXPECT_FALSE(hasText(calls, "example.fl"));
}

TEST(HeaderBar, LayoutPositionsExitButtonInTopRightCorner) {
  EditorContext ctx;
  RecordingRenderer renderer;

  ModuleEditor editor;

  HeaderBar header(renderer, [] {}, [] {}, [] {}, editor);
  header.resize(ctx, kOutputWidth);

  const Rect bounds = header.exitButton().bounds();
  EXPECT_NEAR(bounds.y, ctx.layout.textPad, 1e-6);
  EXPECT_NEAR(bounds.x + bounds.w, kOutputWidth - ctx.layout.textPad, 1e-6);
  EXPECT_GT(bounds.x, 0.0);
  EXPECT_LT(bounds.x + bounds.w, kOutputWidth + 1e-6);
}

TEST(HeaderBar, LayoutPositionsSaveButtonsInTopLeft) {
  EditorContext ctx;
  RecordingRenderer renderer;

  ModuleEditor editor;

  HeaderBar header(renderer, [] {}, [] {}, [] {}, editor);
  header.resize(ctx, kOutputWidth);

  EXPECT_NEAR(header.saveButton().bounds().x, ctx.layout.textPad, 1e-6);
  EXPECT_NEAR(header.saveButton().bounds().y, ctx.layout.textPad, 1e-6);
  EXPECT_GT(header.saveAsButton().bounds().x, header.saveButton().bounds().x + header.saveButton().bounds().w);
}

TEST(HeaderBar, ButtonsSizeThemselvesFromTheirLabels) {
  EditorContext ctx;
  RecordingRenderer renderer;

  ModuleEditor editor;

  HeaderBar header(renderer, [] {}, [] {}, [] {}, editor);
  header.resize(ctx, kOutputWidth);

  EXPECT_GT(header.saveAsButton().bounds().w, header.saveButton().bounds().w)
    << "\"Save As\" is a longer label than \"Save\"";
  EXPECT_GT(header.saveButton().bounds().w, renderer.measureText("Save").x);
}

TEST(HeaderBar, HitTestFindsTheButtonUnderAScreenPoint) {
  EditorContext ctx;
  RecordingRenderer renderer;

  ModuleEditor editor;

  HeaderBar header(renderer, [] {}, [] {}, [] {}, editor);
  header.resize(ctx, kOutputWidth);

  EXPECT_EQ(header.hitTest(header.exitButton().bounds().center()), &header.exitButton());
  EXPECT_EQ(header.hitTest(header.saveButton().bounds().center()), &header.saveButton());
}

TEST(HeaderBar, ButtonsInvokeSaveSaveAsAndExitActions) {
  bool saved = false;
  bool savedAs = false;
  bool exited = false;
  EditorContext ctx;
  RecordingRenderer renderer;

  ModuleEditor editor;

  HeaderBar header(
    renderer, [&saved] { saved = true; }, [&savedAs] { savedAs = true; }, [&exited] { exited = true; }, editor);
  header.resize(ctx, kOutputWidth);

  header.hitTest(header.saveButton().bounds().center())->onClick({});
  EXPECT_TRUE(saved);
  EXPECT_FALSE(savedAs);

  header.hitTest(header.saveAsButton().bounds().center())->onClick({});
  EXPECT_TRUE(savedAs);

  header.hitTest(header.exitButton().bounds().center())->onClick({});
  EXPECT_TRUE(exited);
}

TEST(HeaderBar, ExitButtonZeroSizedBeforeAnyLayout) {
  RecordingRenderer renderer;
  ModuleEditor editor;
  HeaderBar header(renderer, [] {}, [] {}, [] {}, editor);

  // Documents the placeholder pre-layout state: a point can never be "inside"
  // a zero-size Rect (Rect::contains requires strict less-than on the far edge).
  EXPECT_EQ(header.exitButton().bounds(), (Rect{0, 0, 0, 0}));
  EXPECT_FALSE(header.exitButton().bounds().contains(Vec2{0, 0}));
}

TEST(Toolbar, AddingAButtonShiftsTheOthersWithoutTouchingTheToolbar) {
  EditorContext ctx;
  RecordingRenderer renderer;

  ToolbarActor bar(Rect{0, 0, 0, 0}, renderer);
  bar.add("Save", [] {});
  const auto& second = bar.add("Save As", [] {});
  const auto& right = bar.add("Exit", [] {}, ToolbarActor::Align::Right);
  bar.resize(ctx, kOutputWidth);
  const Rect secondBefore = second.bounds();

  ToolbarActor grown(Rect{0, 0, 0, 0}, renderer);
  grown.add("New", [] {});
  grown.add("Save", [] {});
  const auto& secondGrown = grown.add("Save As", [] {});
  const auto& rightGrown = grown.add("Exit", [] {}, ToolbarActor::Align::Right);
  grown.resize(ctx, kOutputWidth);

  EXPECT_GT(secondGrown.bounds().x, secondBefore.x) << "the new button pushes the rest right";
  EXPECT_EQ(rightGrown.bounds(), right.bounds()) << "right-aligned buttons stay flush to the edge";
}
