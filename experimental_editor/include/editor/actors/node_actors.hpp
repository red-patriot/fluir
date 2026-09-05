#pragma once

#include <string>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/actors/actor.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** Actor for a `pt::Binary` node: clicking summarizes the stringified operator. */
  class BinaryActor : public Actor {
   public:
    BinaryActor(fluir::ID functionId, pt::Binary node, Rect bounds) :
      Actor(fluir::FullID{functionId, node.id}, bounds), node_(node) { }

    void onClick(Vec2 worldPos) override;
    void draw(const Subview& body, const EditorContext& ctx) const override;
    PortSet ports(const EditorContext& ctx) const override;

    const std::string& lastClickSummary() const { return lastClickSummary_; }

   private:
    pt::Binary node_;
    std::string lastClickSummary_;
  };

  /** Actor for a `pt::Unary` node: clicking summarizes the stringified operator. */
  class UnaryActor : public Actor {
   public:
    UnaryActor(fluir::ID functionId, pt::Unary node, Rect bounds) :
      Actor(fluir::FullID{functionId, node.id}, bounds), node_(node) { }

    void onClick(Vec2 worldPos) override;
    void draw(const Subview& body, const EditorContext& ctx) const override;
    PortSet ports(const EditorContext& ctx) const override;

    const std::string& lastClickSummary() const { return lastClickSummary_; }

   private:
    pt::Unary node_;
    std::string lastClickSummary_;
  };

  /** Actor for a `pt::Constant` node: clicking summarizes the literal value. */
  class ConstantActor : public Actor {
   public:
    ConstantActor(fluir::ID functionId, pt::Constant node, Rect bounds) :
      Actor(fluir::FullID{functionId, node.id}, bounds), node_(node) { }

    void onClick(Vec2 worldPos) override;
    void draw(const Subview& body, const EditorContext& ctx) const override;
    PortSet ports(const EditorContext& ctx) const override;

    const std::string& lastClickSummary() const { return lastClickSummary_; }

   private:
    pt::Constant node_;
    std::string lastClickSummary_;
  };

  /** Actor for a `pt::Call` node: clicking summarizes the call's target name. */
  class CallActor : public Actor {
   public:
    CallActor(fluir::ID functionId, pt::Call node, Rect bounds) :
      Actor(fluir::FullID{functionId, node.id}, bounds), node_(node) { }

    void onClick(Vec2 worldPos) override;
    void draw(const Subview& body, const EditorContext& ctx) const override;
    PortSet ports(const EditorContext& ctx) const override;

    const std::string& lastClickSummary() const { return lastClickSummary_; }

   private:
    pt::Call node_;
    std::string lastClickSummary_;
  };

}  // namespace fluir::editor
