#pragma once

#include <memory>
#include <vector>

#include "editor/actors/actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"

namespace fluir::editor {

  class GraphScene;

  /** Everything an interaction may touch and the actor tree it dispatches into. */
  struct InteractionContext {
    EditorContext& editor;
    Viewport& view;
    Actor& root;
    Vec2 outputSize;
  };

  class Interaction {
   public:
    virtual ~Interaction() = default;

    /** Return true to consume. Consuming a MouseDown also captures the gesture. */
    virtual bool onEvent(const InputEvent& event, InteractionContext& ctx) = 0;

    virtual bool capturing() const { return false; }

    /** Drops any state pointing into the actor tree. */
    virtual void reset() { }
  };

  /** An ordered set of interactions, first to consume wins. */
  class InteractionChain {
   public:
    void add(std::unique_ptr<Interaction> interaction);
    bool dispatch(const InputEvent& event, InteractionContext& ctx);
    void reset();

   private:
    std::vector<std::unique_ptr<Interaction>> items_;
    Interaction* captured_ = nullptr;
  };

  /** Middle-drag or Space+Left pans; the wheel zooms within the context's clamp. */
  class PanZoomInteraction : public Interaction {
   public:
    bool onEvent(const InputEvent& event, InteractionContext& ctx) override;
    bool capturing() const override { return panning_; }
    void reset() override;

   private:
    bool panning_ = false;
    bool spaceHeld_ = false;
    Vec2 lastPan_;
  };

  /** Left-press on an actor that claims the gesture, then move, then release. */
  class DragInteraction : public Interaction {
   public:
    bool onEvent(const InputEvent& event, InteractionContext& ctx) override;
    bool capturing() const override { return dragActor_ != nullptr; }
    void reset() override { dragActor_ = nullptr; }

   private:
    Actor* dragActor_ = nullptr;
    Vec2 lastDragWorld_;
  };

  /** Left-press sets the scene's selection from whatever was hit; a miss clears it.
   *  Never consumes: the press still reaches drag and click. */
  class SelectionInteraction : public Interaction {
   public:
    explicit SelectionInteraction(GraphScene& scene) : scene_(scene) { }
    bool onEvent(const InputEvent& event, InteractionContext& ctx) override;

   private:
    GraphScene& scene_;
  };

  /** Left-press on an actor that claimed no drag: plain click dispatch. */
  class ClickInteraction : public Interaction {
   public:
    bool onEvent(const InputEvent& event, InteractionContext& ctx) override;
  };

}  // namespace fluir::editor
