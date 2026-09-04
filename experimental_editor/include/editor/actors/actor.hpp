#pragma once

#include "compiler/models/id.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  class Actor {
   public:
    Actor(fluir::ID id, Rect bounds) : id_(id), bounds_(bounds) { }
    virtual ~Actor() = default;
    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;
    Actor(Actor&&) = delete;
    Actor& operator=(Actor&&) = delete;

    fluir::ID id() const { return id_; }
    const Rect& bounds() const { return bounds_; }
    void setBounds(Rect bounds) { bounds_ = bounds; }

    virtual void onClick(Vec2 worldPos) = 0;

   private:
    fluir::ID id_;
    Rect bounds_;
  };

}  // namespace fluir::editor
