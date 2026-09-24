#include "editor/transaction/resize.hpp"

#include <memory>
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

  std::unique_ptr<Transaction> resizeTo(fluir::FullID path, int width, int height) {
    return std::make_unique<ResizeTransaction>(std::move(path), width, height);
  }

}  // namespace fluir::editor
