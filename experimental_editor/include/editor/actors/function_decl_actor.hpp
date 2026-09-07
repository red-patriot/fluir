#pragma once

#include <string>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/actors/actor.hpp"
#include "editor/components/drag_handle.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** Actor for a `pt::FunctionDecl`. */
  class FunctionDeclActor : public Actor {
   public:
    FunctionDeclActor(const pt::FunctionDecl& decl, Rect bounds) :
      Actor(bounds),
      functionId_(decl.id),
      location_(decl.location),
      name_(decl.name),
      drag_(dragRect(location_), location_, this->bounds()) { }

    void onClick(Vec2 worldPos) override;
    bool onDragStart(const EditorContext& ctx, Vec2 worldPos) override;
    void onDrag(const EditorContext& ctx, Vec2 position, Vec2 delta) override;

    void draw(const Subview& frame, const EditorContext& ctx) const override;

    fluir::ID functionId() const { return functionId_; }
    const std::string& lastClickSummary() const { return lastClickSummary_; }

   private:
    fluir::ID functionId_;
    FlowGraphLocation location_;
    std::string name_;
    DragHandle drag_;
    std::string lastClickSummary_;
  };

}  // namespace fluir::editor
