#pragma once

#include <string>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/actors/actor.hpp"
#include "editor/components/container_actor.hpp"
#include "editor/components/drag_handle.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** Actor for a `pt::FunctionDecl`. Owns a body container the function's node
   *  actors hang off, which supplies their offset below the header and clip. */
  class FunctionDeclActor : public Actor {
   public:
    FunctionDeclActor(const pt::FunctionDecl& decl, Rect bounds);

    void layout(const EditorContext& ctx) override;
    void onClick(Vec2 position) override;
    bool onDragStart(const EditorContext& ctx, Vec2 position) override;
    void onDrag(const EditorContext& ctx, Vec2 position, Vec2 delta) override;

    fluir::ID functionId() const { return functionId_; }
    const FlowGraphLocation& location() const { return location_; }
    const std::string& lastClickSummary() const { return lastClickSummary_; }

    ContainerActor& body() { return *body_; }
    const ContainerActor& body() const { return *body_; }

   protected:
    void drawSelf(const Subview& parentView, const EditorContext& ctx) const override;

   private:
    /** The header band across the top of the frame, in parent space. */
    Rect headerRect(const EditorContext& ctx) const;

    fluir::ID functionId_;
    FlowGraphLocation location_;
    std::string name_;
    DragHandle drag_;
    ContainerActor* body_;
    std::string lastClickSummary_;
  };

}  // namespace fluir::editor
