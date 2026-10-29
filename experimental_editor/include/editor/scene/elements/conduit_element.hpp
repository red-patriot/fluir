#ifndef FLUIR_EDITOR_SCENE_ELEMENTS_CONDUIT_ELEMENT_HPP
#define FLUIR_EDITOR_SCENE_ELEMENTS_CONDUIT_ELEMENT_HPP

#include <QGraphicsPathItem>
#include <utility>
#include <vector>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  class ConduitElement : public QGraphicsPathItem {
   public:
    ConduitElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent);

    const FullID& modelPath() const { return path_; }

    void sync(const std::vector<std::pair<Vec2, Vec2>>& wires);

   private:
    FullID path_;
  };

}  // namespace fluir::editor

#endif
