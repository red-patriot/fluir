#pragma once

#include <string>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/actors/node_actor.hpp"
#include "editor/components/drag_handle.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** Actor for a `pt::Binary` node: clicking summarizes the stringified operator. */
  class BinaryActor : public NodeActor {
   public:
    BinaryActor(fluir::ID functionId, pt::Binary node, Rect bound);

    void onClick(Vec2 worldPos) override;
    bool onDragStart(const EditorContext& ctx, Vec2 worldPos) override { return drag_.onDragStart(ctx, worldPos); }
    void onDrag(const EditorContext& ctx, Vec2 worldPos, Vec2 worldDelta) override {
      return drag_.onDrag(ctx, worldPos, worldDelta);
    }

    void draw(const Subview& body, const EditorContext& ctx) const override;
    PortSet ports(const EditorContext& ctx) const override;
    const fluir::FlowGraphLocation& location() const override { return node_.location; }

    const std::string& lastClickSummary() const { return lastClickSummary_; }

   private:
    pt::Binary node_;
    DragHandle drag_;
    std::string lastClickSummary_;
  };

  /** Actor for a `pt::Unary` node: clicking summarizes the stringified operator. */
  class UnaryActor : public NodeActor {
   public:
    UnaryActor(fluir::ID functionId, pt::Unary node, Rect bounds);

    void onClick(Vec2 worldPos) override;
    bool onDragStart(const EditorContext& ctx, Vec2 worldPos) override { return drag_.onDragStart(ctx, worldPos); }
    void onDrag(const EditorContext& ctx, Vec2 worldPos, Vec2 worldDelta) override {
      return drag_.onDrag(ctx, worldPos, worldDelta);
    }

    void draw(const Subview& body, const EditorContext& ctx) const override;
    PortSet ports(const EditorContext& ctx) const override;
    const fluir::FlowGraphLocation& location() const override { return node_.location; }

    const std::string& lastClickSummary() const { return lastClickSummary_; }

   private:
    pt::Unary node_;
    DragHandle drag_;
    std::string lastClickSummary_;
  };

  /** Actor for a `pt::Constant` node: clicking summarizes the literal value. */
  class ConstantActor : public NodeActor {
   public:
    ConstantActor(fluir::ID functionId, pt::Constant node, Rect bounds);

    void onClick(Vec2 worldPos) override;
    bool onDragStart(const EditorContext& ctx, Vec2 worldPos) override { return drag_.onDragStart(ctx, worldPos); }
    void onDrag(const EditorContext& ctx, Vec2 worldPos, Vec2 worldDelta) override {
      return drag_.onDrag(ctx, worldPos, worldDelta);
    }

    void draw(const Subview& body, const EditorContext& ctx) const override;
    PortSet ports(const EditorContext& ctx) const override;
    const fluir::FlowGraphLocation& location() const override { return node_.location; }

    const std::string& lastClickSummary() const { return lastClickSummary_; }

   private:
    pt::Constant node_;
    DragHandle drag_;
    std::string lastClickSummary_;
  };

  /** Actor for a `pt::Call` node: clicking summarizes the call's target name. */
  class CallActor : public NodeActor {
   public:
    CallActor(fluir::ID functionId, pt::Call node, Rect bounds);

    void onClick(Vec2 worldPos) override;
    bool onDragStart(const EditorContext& ctx, Vec2 worldPos) override { return drag_.onDragStart(ctx, worldPos); }
    void onDrag(const EditorContext& ctx, Vec2 worldPos, Vec2 worldDelta) override {
      return drag_.onDrag(ctx, worldPos, worldDelta);
    }

    void draw(const Subview& body, const EditorContext& ctx) const override;
    PortSet ports(const EditorContext& ctx) const override;
    const fluir::FlowGraphLocation& location() const override { return node_.location; }

    const std::string& lastClickSummary() const { return lastClickSummary_; }

   private:
    pt::Call node_;
    DragHandle drag_;
    std::string lastClickSummary_;
  };

}  // namespace fluir::editor
