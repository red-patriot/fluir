#pragma once

#include <string>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/actors/actor.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** Actor for a `pt::FunctionDecl`. */
  class FunctionDeclActor : public Actor {
   public:
    FunctionDeclActor(const pt::FunctionDecl& decl, Rect bounds) :
      Actor(bounds), functionId_(decl.id), location_(decl.location), name_(decl.name) { }

    void onClick(Vec2 worldPos) override;
    void draw(const Subview& frame, const EditorContext& ctx) const override;

    fluir::ID functionId() const { return functionId_; }
    const std::string& lastClickSummary() const { return lastClickSummary_; }

   private:
    fluir::ID functionId_;
    FlowGraphLocation location_;
    std::string name_;
    std::string lastClickSummary_;
  };

}  // namespace fluir::editor
