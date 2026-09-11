#pragma once

#include <cstddef>
#include <optional>
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

    const pt::FunctionDecl::Parameter& parameter() const { return param_; }

    void layout(const EditorContext& ctx) override;
    PortSet ports(const EditorContext& ctx) const override;

   protected:
    void drawSelf(const Subview& body, const EditorContext& ctx) const override;

   private:
    pt::FunctionDecl::Parameter param_;
    std::size_t row_;
  };

  /** A function's return rail, inset from the frame's right edge. */
  class ReturnActor : public PortActor {
   public:
    explicit ReturnActor(const pt::FunctionDecl::Return& ret);

    const pt::FunctionDecl::Return& ret() const { return ret_; }

    /** The owning frame's width in grid units; the frame pushes it down on layout. */
    void setFrameWidth(int widthUnits) { frameWidthUnits_ = widthUnits; }

    void layout(const EditorContext& ctx) override;
    PortSet ports(const EditorContext& ctx) const override;

   protected:
    void drawSelf(const Subview& body, const EditorContext& ctx) const override;

   private:
    pt::FunctionDecl::Return ret_;
    int frameWidthUnits_ = 0;
  };

  /** One conduit: a source port fanning out to zero or more target inputs. */
  class ConduitActor : public Actor {
   public:
    /** One end of a conduit: the port's body-scope id and which input it lands on. */
    struct Endpoint {
      fluir::ID target = fluir::INVALID_ID;
      int index = 0;
    };

    ConduitActor(fluir::ID functionId, const pt::Conduit& conduit);

    /** This actor's conduit, as the parse tree would represent it. */
    pt::Conduit conduit() const;

    void layout(const EditorContext& ctx) override;
    std::optional<fluir::FullID> selectionId() const override { return fluir::FullID{functionId_, id_}; }
    /** A line is not a box: picking a conduit needs distance-to-segment. */
    bool hittable() const override { return false; }

   protected:
    void drawSelf(const Subview& body, const EditorContext& ctx) const override;

   private:
    fluir::ID functionId_;
    fluir::ID id_;
    int index_ = 0;
    fluir::ID sourceId_;
    std::vector<Endpoint> targets_;
    std::vector<std::pair<Vec2, Vec2>> lines_;
  };

}  // namespace fluir::editor
