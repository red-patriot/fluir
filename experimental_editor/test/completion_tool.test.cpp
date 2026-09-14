#include "editor/tools/completion_tool.hpp"

#include <string>
#include <vector>

#include <gtest/gtest.h>

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
