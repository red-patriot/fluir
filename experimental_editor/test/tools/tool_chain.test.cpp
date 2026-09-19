#include <algorithm>
#include <cstddef>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/viewport.hpp"
#include "editor/tools/tool.hpp"
#include "recording_renderer.hpp"
#include "tool_harness.hpp"

// The chain's routing contract: who consumes, who captures the gesture that
// follows, and when capture ends.

namespace {

  using fluir::editor::Box;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::InputEvent;
  using fluir::editor::Rect;
  using fluir::editor::Tool;
  using fluir::editor::ToolChain;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::move;
  using testutil::up;

  const EditorContext kCtx;

  class Probe : public Tool {
   public:
    Probe(bool consumes, bool captures) : consumes_(consumes), captures_(captures) { }

    bool onEvent(const InputEvent& event, EditorState&, std::span<const Box>) override {
      ++events;
      if (event.type == InputEvent::Type::MouseDown && captures_) {
        live_ = true;
      }
      if (event.type == InputEvent::Type::MouseUp) {
        live_ = false;
      }
      return consumes_;
    }
    bool capturing() const override { return live_; }
    void cancel(EditorState&) override {
      ++cancels;
      live_ = false;
    }

    int events = 0;
    int cancels = 0;

   private:
    bool consumes_;
    bool captures_;
    bool live_ = false;
  };

  struct Chain {
    EditorState state{kCtx};
    ToolChain chain;
    Probe* first = nullptr;
    Probe* second = nullptr;

    Chain(Probe a, Probe b) {
      auto pa = std::make_unique<Probe>(a);
      auto pb = std::make_unique<Probe>(b);
      first = pa.get();
      second = pb.get();
      chain.add(std::move(pa));
      chain.add(std::move(pb));
    }

    bool send(const InputEvent& event) { return chain.dispatch(event, state, {}); }
  };

}  // namespace

TEST(ToolChain, TheFirstToolToConsumeWins) {
  Chain c{Probe{true, false}, Probe{true, false}};

  EXPECT_TRUE(c.send(move(Vec2{1, 1})));

  EXPECT_EQ(c.first->events, 1);
  EXPECT_EQ(c.second->events, 0);
}

TEST(ToolChain, AnUnconsumedEventFallsThrough) {
  Chain c{Probe{false, false}, Probe{false, false}};

  EXPECT_FALSE(c.send(move(Vec2{1, 1})));

  EXPECT_EQ(c.first->events, 1);
  EXPECT_EQ(c.second->events, 1);
}

TEST(ToolChain, ACapturingToolGetsEveryEventUntilItReleases) {
  Chain c{Probe{false, false}, Probe{true, true}};

  ASSERT_TRUE(c.send(down(Vec2{1, 1})));
  EXPECT_TRUE(c.send(move(Vec2{2, 2})));
  EXPECT_EQ(c.first->events, 1) << "a captured move must not reach earlier tools";

  EXPECT_TRUE(c.send(up(Vec2{2, 2})));
  c.send(move(Vec2{3, 3}));
  EXPECT_EQ(c.first->events, 2) << "capture ended on release";
}

TEST(ToolChain, CancelDropsCaptureAndCancelsEveryTool) {
  Chain c{Probe{false, false}, Probe{true, true}};
  ASSERT_TRUE(c.send(down(Vec2{1, 1})));

  c.chain.cancel(c.state);

  EXPECT_EQ(c.first->cancels, 1);
  EXPECT_EQ(c.second->cancels, 1);
  c.send(move(Vec2{2, 2}));
  EXPECT_EQ(c.first->events, 2) << "the move reaches the whole chain again";
}

namespace {

  class Painter : public Tool {
   public:
    explicit Painter(Rect mark) : mark_(mark) { }

    bool onEvent(const InputEvent&, EditorState&, std::span<const Box>) override { return false; }
    void draw(const fluir::editor::Subview& view, const EditorState& state, std::span<const Box>) const override {
      view.renderer().fillRect(mark_, state.ctx.theme.text);
    }

   private:
    Rect mark_;
  };

}  // namespace

TEST(ToolChain, AnEarlierToolPaintsOverALaterOne) {
  const Rect first{1, 1, 1, 1};
  const Rect second{2, 2, 2, 2};
  EditorState state{kCtx};
  ToolChain chain;
  chain.add(std::make_unique<Painter>(first));
  chain.add(std::make_unique<Painter>(second));
  testutil::RecordingRenderer r;

  {
    const fluir::editor::Subview view{fluir::editor::Viewport{}, Rect{0, 0, 800, 600}, r};
    chain.draw(view, state, {});
  }

  const std::vector<Rect> fills = testutil::fillsOf(r.calls);
  const auto at = [&](const Rect& want) { return std::ranges::find(fills, want) - fills.begin(); };
  ASSERT_LT(at(first), static_cast<std::ptrdiff_t>(fills.size()));
  ASSERT_LT(at(second), static_cast<std::ptrdiff_t>(fills.size()));
  EXPECT_GT(at(first), at(second)) << "the higher-priority tool paints last";
}
