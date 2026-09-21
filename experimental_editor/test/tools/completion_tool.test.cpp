#include "editor/tools/completion_tool.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/tools/completion_modal.hpp"
#include "recording_renderer.hpp"
#include "tool_harness.hpp"

// single_empty_function.fl: function 1 at world {50,50,500,500}, header 25px so its body starts at y 75.
// simple_binary_expr.fl: same function; binary 1 at world {125,85,25,25}.

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
  constexpr Vec2 kHeader{60, 60};
  constexpr Vec2 kBody{100, 200};

  struct Fixture {
    testutil::RecordingRenderer renderer;
    EditorState state{kCtx};
    CompletionTool uut;

    explicit Fixture(const std::string& file = "read/single_empty_function.fl") {
      state.text = &renderer;
      testutil::loadInto(state, file);
    }
  };

  const CompletionModal* modalOf(const EditorState& state) {
    return dynamic_cast<const CompletionModal*>(state.popup.get());
  }

  // Wheels the open modal until `label`'s row is fully inside its frame, then presses it.
  bool pick(EditorState& state, std::string_view label) {
    auto* modal = dynamic_cast<CompletionModal*>(state.popup.get());
    if (modal == nullptr) {
      return false;
    }
    const auto it = std::ranges::find(modal->labels(), label);
    if (it == modal->labels().end()) {
      return false;
    }
    const auto index = static_cast<std::size_t>(it - modal->labels().begin());
    for (int step = 0; step < 100; ++step) {
      testutil::RecordingRenderer r;
      modal->draw(r, kCtx);
      const std::vector<fluir::editor::Rect> rects = testutil::rectsOf(r.calls);
      std::vector<fluir::editor::Rect> rows;
      for (std::size_t i = 1; i < rects.size(); ++i) {  // rects[0] is the search bar
        if (!(rects[i] == modal->frame())) {
          rows.push_back(rects[i]);
        }
      }
      std::ranges::sort(rows, {}, &fluir::editor::Rect::y);
      const fluir::editor::Rect row = rows.at(index);
      const fluir::editor::Rect frame = modal->frame();
      if (row.y >= frame.y && row.y + row.h <= frame.y + frame.h) {
        return !modal->onEvent(down(row.center()), state);
      }
      const double dir = row.y < frame.y ? 1 : -1;
      modal->onEvent({.type = InputEvent::Type::Wheel, .pos = frame.center(), .wheel = {0, dir}}, state);
    }
    return false;
  }

}  // namespace

TEST(CompletionTool, ARightPressOnTheBackgroundOpensTheTopLevelModalWithoutConsuming) {
  Fixture f;

  EXPECT_FALSE(send(f.uut, f.state, down(kBackground, InputEvent::Button::Right)));

  const auto* modal = dynamic_cast<const CompletionModal*>(f.state.popup.get());
  ASSERT_NE(modal, nullptr);
  EXPECT_EQ(modal->labels(), (std::vector<std::string>{"Function", "Comment"}));
}

TEST(CompletionTool, ARightPressOnAFunctionHeaderOpensNothing) {
  Fixture f;

  EXPECT_FALSE(send(f.uut, f.state, down(kHeader, InputEvent::Button::Right)));

  EXPECT_EQ(f.state.popup, nullptr);
}

TEST(CompletionTool, ARightPressInAFunctionBodyOpensTheBodyModalWithoutConsuming) {
  Fixture f;

  EXPECT_FALSE(send(f.uut, f.state, down(kBody, InputEvent::Button::Right)));

  const auto* modal = modalOf(f.state);
  ASSERT_NE(modal, nullptr);
  const auto& labels = modal->labels();
  EXPECT_NE(std::ranges::find(labels, "+ (binary)"), labels.end());
  EXPECT_NE(std::ranges::find(labels, "Comment"), labels.end());
  EXPECT_EQ(std::ranges::find(labels, "Function"), labels.end());
}

TEST(CompletionTool, ARightPressOnANodeOpensNothing) {
  Fixture f{"read/simple_binary_expr.fl"};

  EXPECT_FALSE(send(f.uut, f.state, down(Vec2{130, 90}, InputEvent::Button::Right)));

  EXPECT_EQ(f.state.popup, nullptr);
}

TEST(CompletionTool, PickingAConstantPlacesItAtBodyLocalUnits) {
  Fixture f;
  f.state.view.pan = Vec2{20, 10};
  const Vec2 screen = kBody + Vec2{20, 10};
  const Vec2 local = (f.state.view.screenToWorld(screen) - Vec2{50, 75}) / kCtx.layout.unitPx;
  send(f.uut, f.state, down(screen, InputEvent::Button::Right));

  ASSERT_TRUE(pick(f.state, "F64"));

  const auto& body = std::get<fluir::pt::FunctionDecl>(f.state.editor.tree().declarations.at(1)).body;
  ASSERT_EQ(body.nodes.size(), 1u);
  const auto* constant = std::get_if<fluir::pt::Constant>(&body.nodes.begin()->second);
  ASSERT_NE(constant, nullptr);
  EXPECT_EQ(constant->location.x, std::lround(local.x));
  EXPECT_EQ(constant->location.y, std::lround(local.y));
  EXPECT_EQ(constant->location.z, 4) << "one above the function's z 3";
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

  send(f.uut, f.state, down(kHeader + Vec2{300, 300}, InputEvent::Button::Right));
  EXPECT_EQ(f.state.popup, nullptr) << "panned header";

  send(f.uut, f.state, down(kHeader, InputEvent::Button::Right));
  ASSERT_NE(modalOf(f.state), nullptr) << "now background";
  EXPECT_NE(std::ranges::find(modalOf(f.state)->labels(), "Function"), modalOf(f.state)->labels().end());
}

TEST(CompletionTool, PickingFunctionPlacesItAtTheRightPressWorldPoint) {
  Fixture f;
  f.state.view.pan = Vec2{20, 10};
  const Vec2 world = f.state.view.screenToWorld(kBackground) / kCtx.layout.unitPx;
  send(f.uut, f.state, down(kBackground, InputEvent::Button::Right));
  ASSERT_NE(f.state.popup, nullptr);

  // The first outlined rect under the search bar is the Function row.
  testutil::RecordingRenderer r;
  f.state.popup->draw(r, kCtx);
  const auto* modal = dynamic_cast<const CompletionModal*>(f.state.popup.get());
  ASSERT_NE(modal, nullptr);
  std::optional<fluir::editor::Rect> functionRow;
  const std::vector<fluir::editor::Rect> rects = testutil::rectsOf(r.calls);
  for (std::size_t i = 1; i < rects.size(); ++i) {  // rects[0] is the search bar
    if (!(rects[i] == modal->frame()) && (!functionRow || rects[i].y < functionRow->y)) {
      functionRow = rects[i];
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

// conditional_empty_scopes.fl: function 1 body origin {0,25}; conditional 2 frame {50,40,500,500},
// then branch {50,40,500,300} (content origin {50,65}), else branch {50,340,500,200} (origin {50,365}).
namespace {
  constexpr Vec2 kThenBranch{100, 100};
  constexpr Vec2 kElseBranch{100, 400};

  const fluir::pt::Conditional& conditionalOf(const EditorState& state) {
    const auto& body = std::get<fluir::pt::FunctionDecl>(state.editor.tree().declarations.at(1)).body;
    return std::get<fluir::pt::Conditional>(body.nodes.at(2));
  }
}  // namespace

TEST(CompletionTool, ARightPressInAThenBranchOpensTheBodyModal) {
  Fixture f{"read/conditional_empty_scopes.fl"};

  EXPECT_FALSE(send(f.uut, f.state, down(kThenBranch, InputEvent::Button::Right)));

  const auto* modal = modalOf(f.state);
  ASSERT_NE(modal, nullptr);
  const auto& labels = modal->labels();
  EXPECT_NE(std::ranges::find(labels, "+ (binary)"), labels.end());
  EXPECT_NE(std::ranges::find(labels, "Comment"), labels.end());
  EXPECT_EQ(std::ranges::find(labels, "Function"), labels.end());
}

TEST(CompletionTool, ARightPressInAnElseBranchOpensTheBodyModal) {
  Fixture f{"read/conditional_empty_scopes.fl"};

  EXPECT_FALSE(send(f.uut, f.state, down(kElseBranch, InputEvent::Button::Right)));

  const auto* modal = modalOf(f.state);
  ASSERT_NE(modal, nullptr);
  const auto& labels = modal->labels();
  EXPECT_NE(std::ranges::find(labels, "+ (binary)"), labels.end());
  EXPECT_NE(std::ranges::find(labels, "Comment"), labels.end());
  EXPECT_EQ(std::ranges::find(labels, "Function"), labels.end());
}

TEST(CompletionTool, ARightPressOnABranchHeaderOpensNothing) {
  Fixture f{"read/conditional_empty_scopes.fl"};

  EXPECT_FALSE(send(f.uut, f.state, down(Vec2{100, 50}, InputEvent::Button::Right)));
  EXPECT_EQ(f.state.popup, nullptr) << "then header band";

  EXPECT_FALSE(send(f.uut, f.state, down(Vec2{100, 350}, InputEvent::Button::Right)));
  EXPECT_EQ(f.state.popup, nullptr) << "else header band";
}

TEST(CompletionTool, PickingAConstantInABranchPlacesItAtBranchLocalUnits) {
  Fixture f{"read/conditional_empty_scopes.fl"};
  send(f.uut, f.state, down(kElseBranch, InputEvent::Button::Right));

  ASSERT_TRUE(pick(f.state, "F64"));

  const fluir::pt::Conditional& conditional = conditionalOf(f.state);
  EXPECT_TRUE(conditional.thenScope->body.nodes.empty());
  const auto& elseNodes = conditional.elseScope->body.nodes;
  ASSERT_EQ(elseNodes.size(), 1u);
  const auto* constant = std::get_if<fluir::pt::Constant>(&elseNodes.begin()->second);
  ASSERT_NE(constant, nullptr);
  EXPECT_EQ(constant->location.x, 10);
  EXPECT_EQ(constant->location.y, 7);
}

TEST(CompletionTool, PickingInABranchHonoursTheViewport) {
  Fixture f{"read/conditional_empty_scopes.fl"};
  f.state.view.pan = Vec2{20, 10};
  send(f.uut, f.state, down(kElseBranch + Vec2{20, 10}, InputEvent::Button::Right));

  ASSERT_TRUE(pick(f.state, "F64"));

  const auto& elseNodes = conditionalOf(f.state).elseScope->body.nodes;
  ASSERT_EQ(elseNodes.size(), 1u);
  const auto* constant = std::get_if<fluir::pt::Constant>(&elseNodes.begin()->second);
  ASSERT_NE(constant, nullptr);
  EXPECT_EQ(constant->location.x, 10);
  EXPECT_EQ(constant->location.y, 7);
}
