#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/actors/actor.hpp"
#include "editor/actors/port_actor.hpp"
#include "editor/actors/rail_actors.hpp"
#include "editor/components/container_actor.hpp"
#include "editor/core/geometry.hpp"
#include "editor/gesture/gesture_host.hpp"
#include "editor/gesture/grip.hpp"

namespace fluir::editor {

  /** Actor for a `pt::FunctionDecl`. Owns a body container the function's node
   *  actors hang off, which supplies their offset below the header and clip. */
  class FunctionDeclActor : public Actor {
   public:
    FunctionDeclActor(const pt::FunctionDecl& decl, Rect bounds);

    void layout(const EditorContext& ctx) override;
    void onClick(Vec2 position) override;
    GestureHost* gestures() override { return &gestures_; }

    /** This frame's location with any live gesture applied. */
    fluir::FlowGraphLocation previewLocation() const { return gestures_.preview(location_); }

    fluir::ID functionId() const { return functionId_; }
    const std::string& name() const { return name_; }

    std::optional<fluir::FullID> selectionId() const override { return fluir::FullID{functionId_}; }
    using Actor::location;  // the const overload, hidden by the override below
    FlowGraphLocation* location() override { return &location_; }
    const std::string& lastClickSummary() const { return lastClickSummary_; }

    /** The frame's width is the frame's state; the return rail needs it to place
     *  itself, so layout pushes it down. */
    void setReturnActor(ReturnActor& ret) { return_ = &ret; }

    ContainerActor& body() { return *body_; }
    const ContainerActor& body() const { return *body_; }

    /** The port `portId` names within this function, or nullptr. */
    PortActor* port(fluir::ID portId) const;
    void registerPort(PortActor& port) { ports_[port.portId()] = &port; }
    void unregisterPort(fluir::ID portId) { ports_.erase(portId); }

   protected:
    void drawSelf(const Subview& parentView, const EditorContext& ctx) const override;
    void drawOverlay(const Subview& parentView, const EditorContext& ctx) const override;

   private:
    /** The header band across the top of the frame, in parent space. */
    Rect headerRect(const EditorContext& ctx) const;

    fluir::ID functionId_;
    FlowGraphLocation location_;
    std::string name_;
    GestureHost gestures_;
    ContainerActor* body_;
    ReturnActor* return_ = nullptr;
    // Conduit endpoints are body-scope ids; the frame owns the body, so it owns the lookup.
    std::unordered_map<fluir::ID, PortActor*> ports_;
    std::string lastClickSummary_;
  };

}  // namespace fluir::editor
