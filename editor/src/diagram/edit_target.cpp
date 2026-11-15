#include "editor/diagram/edit_target.hpp"

#include <string>
#include <utility>

#include "editor/commands/set_field.hpp"
#include "editor/diagram/node.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor::diagram {

  FieldTarget::FieldTarget(FluirNode& node, FieldKey key) : node_(node), key_(std::move(key)) { }

  std::string FieldTarget::current() const { return node_.text(key_()); }

  bool FieldTarget::validate(std::string_view text) const { return node_.accepts(key_(), text); }

  std::vector<std::string> FieldTarget::options() const { return node_.choices(key_()); }

  void FieldTarget::commit(std::string_view text) {
    std::string before = current();
    if (text == before || !validate(text)) {
      return;
    }
    ModuleScene::of(node_).undoStack().push(new SetField(node_, key_(), std::move(before), std::string(text)));
  }

}  // namespace fluir::editor::diagram
