#ifndef FLUIR_EDITOR_SCENE_MODULE_SCENE_HPP
#define FLUIR_EDITOR_SCENE_MODULE_SCENE_HPP

#include <QGraphicsScene>

namespace fluir::editor {

  /** A module's items: the scene's item tree is the program. */
  class ModuleScene : public QGraphicsScene {
   public:
    using QGraphicsScene::QGraphicsScene;
  };

}  // namespace fluir::editor

#endif
