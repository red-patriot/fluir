#pragma once

#include <string>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/location.hpp"
#include "editor/actors/actor.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** Actor for a `pt::FunctionDecl`. */
  class FunctionDeclActor : public Actor {
   public:
    FunctionDeclActor(const pt::FunctionDecl& decl, Rect bounds) :
      Actor(fluir::FullID{decl.id}, bounds), location_(decl.location), name_(decl.name) { }

    void onClick(Vec2 worldPos) override;
    void draw(const Subview& frame, const EditorContext& ctx) const override;
    PortSet ports(const EditorContext& ctx) const override;

    const std::string& lastClickSummary() const { return lastClickSummary_; }

   private:
    FlowGraphLocation location_;
    std::string name_;
    std::string lastClickSummary_;
  };

}  // namespace fluir::editor
