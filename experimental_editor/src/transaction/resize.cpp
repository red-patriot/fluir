#include "editor/transaction/resize.hpp"

#include <utility>

#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  bool ResizeTransaction::execute(pt::ParseTree& tree) {
    FlowGraphLocation* location = locationAt(tree, path_);
    if (location == nullptr || (location->width == width_ && location->height == height_)) {
      return false;
    }
    std::swap(location->width, width_);
    std::swap(location->height, height_);
    return true;
  }

}  // namespace fluir::editor
