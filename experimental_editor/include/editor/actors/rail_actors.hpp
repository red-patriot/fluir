#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/actors/actor.hpp"
#include "editor/actors/port_actor.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** One row of a function's parameter rail, down the body's left edge. */
  class ParameterActor : public PortActor {
   public:
    ParameterActor(const pt::FunctionDecl::Parameter& param, std::size_t row);

    void layout(const EditorContext& ctx) override;
    PortSet ports(const EditorContext& ctx) const override;

   protected:
    void drawSelf(const Subview& body, const EditorContext& ctx) const override;

   private:
    std::string label_;
    std::size_t row_;
  };

  /** A function's return rail, inset from the frame's right edge. */
  class ReturnActor : public PortActor {
   public:
    explicit ReturnActor(const pt::FunctionDecl::Return& ret);

    /** The owning frame's width in grid units; the frame pushes it down on layout. */
    void setFrameWidth(int widthUnits) { frameWidthUnits_ = widthUnits; }

    void layout(const EditorContext& ctx) override;
    PortSet ports(const EditorContext& ctx) const override;

   protected:
    void drawSelf(const Subview& body, const EditorContext& ctx) const override;

   private:
    std::string typeName_;
    int frameWidthUnits_ = 0;
  };

  /** One conduit: a source port fanning out to zero or more target inputs. */
  class ConduitActor : public Actor {
   public:
    struct Target {
      PortActor* actor;
      int index;
    };

    ConduitActor(PortActor& source, std::vector<Target> targets);

    void layout(const EditorContext& ctx) override;
    /** A line is not a box: picking a conduit needs distance-to-segment. */
    bool hittable() const override { return false; }

   protected:
    void drawSelf(const Subview& body, const EditorContext& ctx) const override;

   private:
    PortActor* source_;
    std::vector<Target> targets_;
    std::vector<std::pair<Vec2, Vec2>> lines_;
  };

}  // namespace fluir::editor
