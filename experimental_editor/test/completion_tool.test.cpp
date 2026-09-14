#include "editor/tools/completion_tool.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/tools/completion_modal.hpp"
#include "recording_renderer.hpp"
#include "tool_harness.hpp"

// single_empty_function.fl: function 1 at world {50,50,500,500}.

namespace {

  using fluir::editor::CompletionModal;
  using fluir::editor::CompletionTool;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::InputEvent;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::send;

  const EditorContext kCtx;
  constexpr Vec2 kBackground{700, 700};
  constexpr Vec2 kBody{60, 60};

  struct Fixture {
    testutil::RecordingRenderer renderer;
    EditorState state{kCtx};
    CompletionTool uut;

    Fixture() {
      state.text = &renderer;
      testutil::loadInto(state, "read/single_empty_function.fl");
    }
  };

}  // namespace

TEST(CompletionTool, ARightPressOnTheBackgroundOpensTheTopLevelModalWithoutConsuming) {
  Fixture f;

  EXPECT_FALSE(send(f.uut, f.state, down(kBackground, InputEvent::Button::Right)));

  const auto* modal = dynamic_cast<const CompletionModal*>(f.state.popup.get());
  ASSERT_NE(modal, nullptr);
  EXPECT_EQ(modal->labels(), (std::vector<std::string>{"Function", "Comment"}));
}

TEST(CompletionTool, ARightPressOnAFunctionBodyOpensNothing) {
  Fixture f;

  EXPECT_FALSE(send(f.uut, f.state, down(kBody, InputEvent::Button::Right)));

  EXPECT_EQ(f.state.popup, nullptr);
}

TEST(CompletionTool, LeftAndMiddlePressesOpenNothing) {
  Fixture f;

  EXPECT_FALSE(send(f.uut, f.state, down(kBackground)));
  EXPECT_FALSE(send(f.uut, f.state, down(kBackground, InputEvent::Button::Middle)));

  EXPECT_EQ(f.state.popup, nullptr);
}

TEST(CompletionTool, PressesHonourTheViewport) {
  Fixture f;
  f.state.view.pan = Vec2{300, 300};

  send(f.uut, f.state, down(kBody + Vec2{300, 300}, InputEvent::Button::Right));
  EXPECT_EQ(f.state.popup, nullptr) << "panned body";

  send(f.uut, f.state, down(kBody, InputEvent::Button::Right));
  EXPECT_NE(f.state.popup, nullptr) << "now background";
}

TEST(CompletionTool, PickingFunctionPlacesItAtTheRightPressWorldPoint) {
  Fixture f;
  f.state.view.pan = Vec2{20, 10};
  const Vec2 world = f.state.view.screenToWorld(kBackground) / kCtx.layout.unitPx;
  send(f.uut, f.state, down(kBackground, InputEvent::Button::Right));
  ASSERT_NE(f.state.popup, nullptr);

  // The first outlined rect under the frame's top is the Function row.
  testutil::RecordingRenderer r;
  f.state.popup->draw(r, kCtx);
  const auto* modal = dynamic_cast<const CompletionModal*>(f.state.popup.get());
  ASSERT_NE(modal, nullptr);
  std::optional<fluir::editor::Rect> functionRow;
  for (const fluir::editor::Rect& rect : testutil::rectsOf(r.calls)) {
    if (!(rect == modal->frame()) && (!functionRow || rect.y < functionRow->y)) {
      functionRow = rect;
    }
  }
  ASSERT_TRUE(functionRow.has_value());
  const std::size_t before = f.state.editor.tree().declarations.size();

  EXPECT_FALSE(f.state.popup->onEvent(down(functionRow->center()), f.state));

  const auto& decls = f.state.editor.tree().declarations;
  ASSERT_EQ(decls.size(), before + 1);
  const auto newest = std::ranges::max_element(decls, {}, [](const auto& kv) { return kv.first; });
  const auto* fn = std::get_if<fluir::pt::FunctionDecl>(&newest->second);
  ASSERT_NE(fn, nullptr);
  EXPECT_EQ(fn->location.x, std::lround(world.x));
  EXPECT_EQ(fn->location.y, std::lround(world.y));
}
