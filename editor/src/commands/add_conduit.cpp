#include "editor/commands/add_conduit.hpp"

#include <QGraphicsScene>

#include "editor/diagram/ids.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor {

  AddConduit::AddConduit(diagram::Scope& scope, diagram::Terminal& source, diagram::Terminal& sink) :
    scope_(scope),
    source_(source),
    sink_(sink),
    id_(diagram::nextId(scope)),
    cut_({&scope}, [&sink](const diagram::Terminal* terminal) { return terminal == &sink; }) { }

  void AddConduit::redo() {
    cut_.apply();
    if (!conduit_) {
      conduit_ = new diagram::Conduit(id_, &source_, {&sink_}, ModuleScene::of(scope_).context(), &scope_);
    } else {
      taken_.release()->setParentItem(&scope_);
      conduit_->attach();
    }
  }

  void AddConduit::undo() {
    conduit_->detach();
    scope_.scene()->removeItem(conduit_);
    taken_.reset(conduit_);
    cut_.restore();
  }

}  // namespace fluir::editor
