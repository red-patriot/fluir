#pragma once

#include <memory>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/gesture/gesture.hpp"
#include "editor/gesture/grip.hpp"

namespace testutil {

  /** What a stub actor's gesture was handed, for tests that drive a real
   *  DragInteraction but do not care what the gesture would edit. */
  struct GestureLog {
    fluir::editor::Vec2 delta;
    int starts = 0;
    int commits = 0;
    int cancels = 0;
  };

  /** Reports into a GestureLog and edits nothing. */
  class LoggingGesture : public fluir::editor::Gesture {
   public:
    explicit LoggingGesture(GestureLog& log) : log_(log) { ++log_.starts; }
    ~LoggingGesture() override {
      if (!committed_) {
        ++log_.cancels;
      }
    }

    LoggingGesture(const LoggingGesture&) = delete;
    LoggingGesture& operator=(const LoggingGesture&) = delete;
    LoggingGesture(LoggingGesture&&) = delete;
    LoggingGesture& operator=(LoggingGesture&&) = delete;

    void update(const fluir::editor::EditorContext&, fluir::editor::Vec2 worldDelta) override {
      log_.delta = log_.delta + worldDelta;
    }

    fluir::FlowGraphLocation preview(const fluir::FlowGraphLocation& loc) const override { return loc; }

    void commit(const fluir::editor::EditorContext&, const fluir::FlowGraphLocation&, const fluir::FullID&) override {
      ++log_.commits;
      committed_ = true;
    }

   private:
    GestureLog& log_;
    bool committed_ = false;
  };

  /** A grip at `gridRect` within its actor, reporting into `log`. The default
   *  covers the whole actor; presses are already narrowed to it by hit-testing. */
  inline fluir::editor::Grip loggingGrip(GestureLog& log,
                                         fluir::editor::Rect gridRect = fluir::editor::Rect{0, 0, 1e6, 1e6}) {
    return fluir::editor::Grip{.rect = [gridRect](const fluir::FlowGraphLocation&) { return gridRect; },
                               .begin = [&log] { return std::make_unique<LoggingGesture>(log); },
                               .frame = fluir::editor::actorBounds};
  }

}  // namespace testutil
