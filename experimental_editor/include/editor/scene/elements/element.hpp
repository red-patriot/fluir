#ifndef FLUIR_EDITOR_SCENE_ELEMENTS_ELEMENT_HPP
#define FLUIR_EDITOR_SCENE_ELEMENTS_ELEMENT_HPP

#include <optional>
#include <QGraphicsObject>
#include <QRectF>
#include <utility>
#include <vector>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/tree.hpp"
#include "editor/view/draw/draw_utils.hpp"
#include "editor/view/graph_layout.hpp"

namespace fluir::editor {

  /** An element representing a Fluir Node on a flow graph display. */
  class Element : public QGraphicsObject {
   public:
    const FullID& modelPath() const { return path_; }
    QRectF boundingRect() const override { return rect_; }

   protected:
    Element(FullID path, const EditorContext& ctx, QGraphicsItem* parent);

    void place(const Rect& world);
    void clearComponents();

    template <typename T, typename... Args>
    T* addComponent(Args&&... args) {
      return new T(std::forward<Args>(args)..., components_);
    }

    void addLabels(const et::ParseTree& tree, const FullID& fieldPath, const std::vector<FieldLabel>& labels);
    void addTerminals(const FullID& endpointPath, const TerminalSet& set);
    void addGrips(const Rect& world, std::optional<Part> resize, const Rect& moveGrip);

    const EditorContext& ctx_;

   private:
    FullID path_;
    QRectF rect_;
    QGraphicsItem* components_;
  };

}  // namespace fluir::editor

#endif
