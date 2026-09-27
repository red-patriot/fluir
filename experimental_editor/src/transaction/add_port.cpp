#include "editor/transaction/add_port.hpp"

#include <algorithm>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  namespace {

    et::Conditional* conditionalAt(et::ParseTree& tree, const fluir::FullID& path) {
      return std::get_if<et::Conditional>(nodeAt(tree, path));
    }

  }  // namespace

  bool AddPort::execute(et::ParseTree& tree) {
    et::Conditional* conditional = conditionalAt(tree, path_);
    if (!conditional || innerId_ == INVALID_ID || conditional->condition.innerId == innerId_) {
      return false;
    }
    const auto taken = [this](const et::BlockPort& port) { return port.innerId == innerId_; };
    if (std::ranges::any_of(conditional->inputs, taken) || std::ranges::any_of(conditional->outputs, taken)) {
      return false;
    }
    (output_ ? conditional->outputs : conditional->inputs).push_back({.innerId = innerId_, .y = y_});
    return true;
  }

  bool AddPort::unexecute(et::ParseTree& tree) {
    et::Conditional* conditional = conditionalAt(tree, path_);
    if (!conditional) {
      return false;
    }
    std::vector<et::BlockPort>& ports = output_ ? conditional->outputs : conditional->inputs;
    if (ports.empty() || ports.back().innerId != innerId_) {
      return false;
    }
    ports.pop_back();
    return true;
  }

  std::unique_ptr<Transaction> addPort(fluir::FullID path, bool output, fluir::ID innerId, int y) {
    return std::make_unique<AddPort>(std::move(path), output, innerId, y);
  }

}  // namespace fluir::editor
