#include "editor/tools/popup_tool.hpp"

#include <memory>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/viewport.hpp"
#include "editor/tools/popup.hpp"
#include "recording_renderer.hpp"
#include "tool_harness.hpp"

// The host routes every event to an open popup, consumes it, and drops the popup once it asks to close.

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::InputEvent;
  using fluir::editor::Popup;
  using fluir::editor::PopupTool;
  using fluir::editor::Rect;
  using fluir::editor::Renderer;
  using fluir::editor::Subview;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::down;
  using testutil::move;

  const EditorContext kCtx;
  const Rect kMarker{1, 2, 3, 4};

  struct Probe : Popup {
    explicit Probe(int& events, bool stayOpen = true) : events_(events), stayOpen_(stayOpen) { }

    bool onEvent(const InputEvent&, EditorState&) override {
      ++events_;
      return stayOpen_;
    }
    void draw(Renderer& r, const EditorContext& ctx) const override { r.fillRect(kMarker, ctx.theme.text); }

   private:
    int& events_;
    bool stayOpen_;
  };

}  // namespace

TEST(PopupTool, WithNoPopupEventsFallThrough) {
  EditorState state{kCtx};
  PopupTool uut;

  EXPECT_FALSE(testutil::send(uut, state, down(Vec2{1, 1})));
}

TEST(PopupTool, AnOpenPopupGetsAndConsumesEveryEvent) {
  EditorState state{kCtx};
  int events = 0;
  state.popup = std::make_unique<Probe>(events);
  PopupTool uut;

  EXPECT_TRUE(testutil::send(uut, state, down(Vec2{1, 1})));
  EXPECT_TRUE(testutil::send(uut, state, testutil::key(InputEvent::Key::Delete)));

  EXPECT_EQ(events, 2);
  EXPECT_NE(state.popup, nullptr);
}

TEST(PopupTool, APopupThatAsksToCloseIsDroppedAndItsEventStillConsumed) {
  EditorState state{kCtx};
  int events = 0;
  state.popup = std::make_unique<Probe>(events, false);
  PopupTool uut;

  EXPECT_TRUE(testutil::send(uut, state, move(Vec2{1, 1})));

  EXPECT_EQ(state.popup, nullptr);
  EXPECT_FALSE(testutil::send(uut, state, move(Vec2{2, 2})));
}

TEST(PopupTool, CancelDropsThePopup) {
  EditorState state{kCtx};
  int events = 0;
  state.popup = std::make_unique<Probe>(events);
  PopupTool uut;

  uut.cancel(state);

  EXPECT_EQ(state.popup, nullptr);
}

TEST(PopupTool, DrawForwardsToTheOpenPopup) {
  EditorState state{kCtx};
  int events = 0;
  testutil::RecordingRenderer r;
  PopupTool uut;
  {
    const Subview view{Viewport{}, Rect{0, 0, 800, 600}, r};
    uut.draw(view, state, {});
  }
  EXPECT_FALSE(testutil::hasFill(r.calls, kMarker)) << "nothing open, nothing drawn";

  state.popup = std::make_unique<Probe>(events);
  {
    const Subview view{Viewport{}, Rect{0, 0, 800, 600}, r};
    uut.draw(view, state, {});
  }
  EXPECT_TRUE(testutil::hasFill(r.calls, kMarker));
}
