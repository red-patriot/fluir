#include "editor/tools/menu_popup.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/components/menu.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/tools/tool.hpp"
#include "recording_renderer.hpp"
#include "tool_harness.hpp"

// A menu popup picks the pressed row and closes; any other press or Escape closes without picking.

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::InputEvent;
  using fluir::editor::layoutMenu;
  using fluir::editor::MenuLayout;
  using fluir::editor::MenuPopup;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::key;
  using testutil::move;

  const EditorContext kCtx;
  const std::vector<std::string> kLabels{"+", "-", "*"};
  const Rect kAnchor{100, 100, 25, 25};
  const Rect kBounds{0, 0, 800, 600};

  struct Fixture {
    EditorState state{kCtx};
    std::optional<std::size_t> picked;
    MenuPopup uut{kLabels, kAnchor, kBounds, kCtx.layout, nullptr, [this](std::size_t i, EditorState&) { picked = i; }};
    MenuLayout layout = layoutMenu(kLabels, kAnchor, kBounds, kCtx.layout, nullptr);
  };

}  // namespace

TEST(MenuPopup, PressingARowPicksItAndCloses) {
  Fixture f;

  EXPECT_FALSE(f.uut.onEvent(down(f.layout.items[2].center()), f.state));

  EXPECT_EQ(f.picked, 2u);
}

TEST(MenuPopup, PressingOutsideClosesWithoutPicking) {
  Fixture f;

  EXPECT_FALSE(f.uut.onEvent(down(Vec2{700, 500}), f.state));

  EXPECT_FALSE(f.picked.has_value());
}

TEST(MenuPopup, AnyButtonOutsideCloses) {
  Fixture f;

  EXPECT_FALSE(f.uut.onEvent(down(Vec2{700, 500}, InputEvent::Button::Right), f.state));
}

TEST(MenuPopup, EscapeClosesWithoutPicking) {
  Fixture f;

  EXPECT_FALSE(f.uut.onEvent(key(InputEvent::Key::Escape), f.state));

  EXPECT_FALSE(f.picked.has_value());
}

TEST(MenuPopup, MovesReleasesAndOtherKeysKeepItOpen) {
  Fixture f;

  EXPECT_TRUE(f.uut.onEvent(move(f.layout.items[1].center()), f.state));
  EXPECT_TRUE(f.uut.onEvent(testutil::up(Vec2{700, 500}), f.state));
  EXPECT_TRUE(f.uut.onEvent(key(InputEvent::Key::Delete), f.state));
  EXPECT_TRUE(f.uut.onEvent(testutil::text("x"), f.state));

  EXPECT_FALSE(f.picked.has_value());
}

TEST(MenuPopup, HoveringARowHighlightsIt) {
  Fixture f;
  testutil::RecordingRenderer r;

  f.uut.onEvent(move(f.layout.items[1].center()), f.state);
  f.uut.draw(r, kCtx);

  EXPECT_TRUE(testutil::hasFill(r.calls, f.layout.items[1]));
  EXPECT_EQ(f.uut.labels(), kLabels);
}

namespace {

  struct DisabledFixture {
    EditorState state{kCtx};
    std::optional<std::size_t> picked;
    MenuPopup uut{kLabels,
                  kAnchor,
                  kBounds,
                  kCtx.layout,
                  nullptr,
                  [this](std::size_t i, EditorState&) { picked = i; },
                  {true, false}};
    MenuLayout layout = layoutMenu(kLabels, kAnchor, kBounds, kCtx.layout, nullptr);
  };

}  // namespace

TEST(MenuPopup, PressingADisabledRowIsConsumedWithoutPicking) {
  DisabledFixture f;

  EXPECT_TRUE(f.uut.onEvent(down(f.layout.items[1].center()), f.state));

  EXPECT_FALSE(f.picked.has_value());
}

TEST(MenuPopup, RowsPastTheEnabledListStayEnabled) {
  DisabledFixture f;

  EXPECT_FALSE(f.uut.onEvent(down(f.layout.items[2].center()), f.state));

  EXPECT_EQ(f.picked, 2u);
}

TEST(MenuPopup, HoveringADisabledRowHighlightsNothing) {
  DisabledFixture f;
  testutil::RecordingRenderer r;

  f.uut.onEvent(move(f.layout.items[1].center()), f.state);
  f.uut.draw(r, kCtx);

  for (const Rect& row : f.layout.items) {
    EXPECT_FALSE(testutil::hasFill(r.calls, row));
  }
}
