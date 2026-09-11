#include "editor/transaction/delete_function.hpp"

#include <utility>

namespace fluir::editor {

  bool DeleteFunctionTransaction::execute(GraphScene& scene) {
    removed_ = scene.detach(fluir::FullID{functionId_});
    return removed_.actor != nullptr;
  }

  bool DeleteFunctionTransaction::unexecute(GraphScene& scene) {
    if (removed_.actor == nullptr) {
      return false;
    }
    return scene.attach(std::move(removed_));
  }

}  // namespace fluir::editor
