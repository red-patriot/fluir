#ifndef FLUIR_EDITOR_DIAGRAM_CONTEXT_MENU_HPP
#define FLUIR_EDITOR_DIAGRAM_CONTEXT_MENU_HPP

#include <functional>
#include <memory>
#include <QAction>
#include <QMenu>
#include <QObject>
#include <QPoint>
#include <QString>
#include <utility>

namespace fluir::editor::diagram {

  /** Adds an entry to `menu` that runs `onPick`. */
  inline QAction* addEntry(QMenu& menu, const QString& text, std::function<void()> onPick) {
    QAction* action = menu.addAction(text);
    QObject::connect(action, &QAction::triggered, &menu, [onPick = std::move(onPick)] { onPick(); });
    return action;
  }

  /** Pops `menu` up at `screenPos` without blocking; it deletes itself once closed. */
  inline void popUp(std::unique_ptr<QMenu> menu, QPoint screenPos) {
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu.release()->popup(screenPos);
  }

}  // namespace fluir::editor::diagram

#endif
