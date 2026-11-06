#include "editor/diagram/edits.hpp"

#include <memory>
#include <QMenu>
#include <QString>
#include <string>
#include <utility>

#include "editor/diagram/context_menu.hpp"

namespace fluir::editor::diagram {

  void pick(EditTarget& target, QPoint screenPos) {
    auto menu = std::make_unique<QMenu>();
    for (const std::string& option : target.options()) {
      addEntry(*menu, QString::fromStdString(option), [&target, option] { target.commit(option); });
    }
    popUp(std::move(menu), screenPos);
  }

  void flip(EditTarget& target) { target.commit(target.current() == "true" ? "false" : "true"); }

}  // namespace fluir::editor::diagram
