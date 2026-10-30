#ifndef FLUIR_EDITOR_SCENE_MODULE_SCENE_HPP
#define FLUIR_EDITOR_SCENE_MODULE_SCENE_HPP

#include <QGraphicsScene>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/editor_context.hpp"

namespace fluir::editor {

  /** A module's data-flow diagram. */
  class ModuleScene : public QGraphicsScene {
   public:
    explicit ModuleScene(const EditorContext& ctx, QObject* parent = nullptr) : QGraphicsScene(parent), ctx_(ctx) { }

    const EditorContext& context() const { return ctx_; }

    const pt::Header& header() const { return header_; }
    void setHeader(const pt::Header& header) { header_ = header; }

   private:
    const EditorContext& ctx_;
    pt::Header header_;
  };

}  // namespace fluir::editor

#endif
