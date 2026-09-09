#pragma once

#include <functional>
#include <string>
#include <utility>

#include "editor/actors/actor.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** A clickable labeled button: no fluir id, purely UI chrome. */
  class ButtonActor : public Actor {
   public:
    ButtonActor(std::string label, std::function<void()> action, Rect bounds) :
      Actor(bounds), label_(std::move(label)), action_(std::move(action)) { }

    void onClick(Vec2 position) override;

    const std::string& label() const { return label_; }

   protected:
    void drawSelf(const Subview& body, const EditorContext& ctx) const override;

   private:
    std::string label_;
    std::function<void()> action_;
  };

}  // namespace fluir::editor
