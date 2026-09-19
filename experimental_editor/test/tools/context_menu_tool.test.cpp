#include "editor/tools/context_menu_tool.hpp"

#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/components/menu.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/tools/menu_popup.hpp"
#include "editor/tools/popup_tool.hpp"
#include "recording_renderer.hpp"
#include "tool_harness.hpp"

// simple_binary_expr.fl: function 1 at world {50,50,500,500}; binary 1 at world {125,85,25,25}.

namespace {

  using fluir::FullID;
  using fluir::editor::Box;
  using fluir::editor::ContextMenuTool;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::InputEvent;
  using fluir::editor::layoutMenu;
  using fluir::editor::MenuItem;
  using fluir::editor::MenuLayout;
  using fluir::editor::MenuPopup;
  using fluir::editor::MenuProvider;
  using fluir::editor::PopupTool;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::send;

  const EditorContext kCtx;
  constexpr Vec2 kNodeBody{137, 97};
  constexpr Vec2 kBackground{700, 700};
  const std::vector<std::string> kLabels{"First", "Second item"};

  /** Records its calls; hands out `labels` whose actions log their index. */
  struct FakeProvider {
    std::vector<std::string> labels;
    int calls = 0;
    std::optional<FullID> path;
    std::optional<Vec2> world;
    std::vector<int> ran;

    MenuProvider provider() {
      return [this](const Box& hit, Vec2 at, const EditorState&) {
        ++calls;
        path = hit.path;
        world = at;
        std::vector<MenuItem> items;
        for (std::size_t i = 0; i < labels.size(); ++i) {
          items.push_back(MenuItem{labels[i], [this, i](EditorState&) { ran.push_back(static_cast<int>(i)); }});
        }
        return items;
      };
    }
  };

  struct Fixture {
    EditorState state{kCtx};

    Fixture() { testutil::loadInto(state, "read/simple_binary_expr.fl"); }
  };

  std::vector<std::string> menuLabels(const EditorState& state) {
    const auto* menu = dynamic_cast<const MenuPopup*>(state.popup.get());
    return menu == nullptr ? std::vector<std::string>{} : menu->labels();
  }

  MenuLayout layoutAt(Vec2 press, const std::vector<std::string>& labels, const EditorState& state) {
    return layoutMenu(labels, Rect{press.x, press.y, 0, 0}, popupBounds(state), kCtx.layout, state.text);
  }

}  // namespace

TEST(ContextMenuTool, ARightPressOnANodeOpensTheProvidersItemsAndConsumes) {
  Fixture f;
  FakeProvider p{.labels = kLabels};
  ContextMenuTool uut{{p.provider()}};

  EXPECT_TRUE(send(uut, f.state, down(kNodeBody, InputEvent::Button::Right)));

  EXPECT_EQ(menuLabels(f.state), kLabels);
}

TEST(ContextMenuTool, TheMenuOpensAtTheCursor) {
  Fixture f;
  FakeProvider p{.labels = kLabels};
  ContextMenuTool uut{{p.provider()}};

  send(uut, f.state, down(kNodeBody, InputEvent::Button::Right));

  ASSERT_NE(f.state.popup, nullptr);
  testutil::RecordingRenderer r;
  f.state.popup->draw(r, kCtx);
  const std::vector<Rect> outlines = testutil::rectsOf(r.calls);
  ASSERT_FALSE(outlines.empty());
  testutil::expectVecNear(outlines.front().topLeft(), kNodeBody);
}

TEST(ContextMenuTool, PickingARowRunsOnlyItsActionAndCloses) {
  Fixture f;
  FakeProvider p{.labels = kLabels};
  ContextMenuTool uut{{p.provider()}};
  PopupTool host;
  send(uut, f.state, down(kNodeBody, InputEvent::Button::Right));
  const MenuLayout menu = layoutAt(kNodeBody, kLabels, f.state);

  EXPECT_TRUE(send(host, f.state, down(menu.items[1].center())));

  EXPECT_EQ(p.ran, (std::vector<int>{1}));
  EXPECT_EQ(f.state.popup, nullptr);
}

TEST(ContextMenuTool, EscapeOrAPressOutsideClosesWithoutRunningAnything) {
  Fixture f;
  FakeProvider p{.labels = kLabels};
  ContextMenuTool uut{{p.provider()}};
  PopupTool host;

  send(uut, f.state, down(kNodeBody, InputEvent::Button::Right));
  send(host, f.state, testutil::key(InputEvent::Key::Escape));
  EXPECT_EQ(f.state.popup, nullptr);

  send(uut, f.state, down(kNodeBody, InputEvent::Button::Right));
  send(host, f.state, down(kBackground));
  EXPECT_EQ(f.state.popup, nullptr);

  EXPECT_TRUE(p.ran.empty());
}

TEST(ContextMenuTool, LeftPressesBackgroundAndEmptyProvidersOpenNothing) {
  Fixture f;
  FakeProvider p{.labels = kLabels};
  FakeProvider empty;
  ContextMenuTool uut{{p.provider()}};
  ContextMenuTool nothing{{empty.provider()}};

  EXPECT_FALSE(send(uut, f.state, down(kNodeBody)));
  EXPECT_FALSE(send(uut, f.state, down(kBackground, InputEvent::Button::Right)));
  EXPECT_FALSE(send(nothing, f.state, down(kNodeBody, InputEvent::Button::Right)));

  EXPECT_EQ(f.state.popup, nullptr);
}

TEST(ContextMenuTool, TheFirstProviderWithItemsWins) {
  Fixture f;
  FakeProvider empty;
  FakeProvider first{.labels = {"A"}};
  FakeProvider later{.labels = {"B"}};
  ContextMenuTool uut{{empty.provider(), first.provider(), later.provider()}};

  send(uut, f.state, down(kNodeBody, InputEvent::Button::Right));

  EXPECT_EQ(menuLabels(f.state), (std::vector<std::string>{"A"}));
  EXPECT_EQ(empty.calls, 1);
  EXPECT_EQ(later.calls, 0);
}

TEST(ContextMenuTool, ProvidersGetTheHitPathAndWorldPointThroughTheViewport) {
  Fixture f;
  f.state.view.pan = Vec2{40, -30};
  f.state.view.scale = 2.0;
  FakeProvider p{.labels = kLabels};
  ContextMenuTool uut{{p.provider()}};

  send(uut, f.state, down(f.state.view.worldToScreen(kNodeBody), InputEvent::Button::Right));

  EXPECT_EQ(p.path, (FullID{1, 1}));
  ASSERT_TRUE(p.world.has_value());
  testutil::expectVecNear(*p.world, kNodeBody);
}

TEST(ContextMenuTool, PickingADisabledRowRunsNothingAndKeepsTheMenu) {
  Fixture f;
  std::vector<int> ran;
  MenuProvider provider = [&ran](const Box&, Vec2, const EditorState&) {
    return std::vector<MenuItem>{
      MenuItem{.label = "Off", .onClick = [&ran](EditorState&) { ran.push_back(0); }, .enabled = false},
      MenuItem{.label = "On", .onClick = [&ran](EditorState&) { ran.push_back(1); }}};
  };
  ContextMenuTool uut{{provider}};
  PopupTool host;
  send(uut, f.state, down(kNodeBody, InputEvent::Button::Right));
  const MenuLayout menu = layoutAt(kNodeBody, {"Off", "On"}, f.state);

  send(host, f.state, down(menu.items[0].center()));

  EXPECT_TRUE(ran.empty());
  EXPECT_NE(f.state.popup, nullptr);
}
