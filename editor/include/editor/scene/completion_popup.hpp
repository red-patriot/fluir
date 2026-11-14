#ifndef FLUIR_EDITOR_SCENE_COMPLETION_POPUP_HPP
#define FLUIR_EDITOR_SCENE_COMPLETION_POPUP_HPP

#include <functional>
#include <memory>
#include <QEvent>
#include <QModelIndex>
#include <QObject>
#include <QPoint>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QWidget>

namespace Ui {
  class completionPopup;
}

namespace fluir::editor {

  /** A popup that shows completions at a given location on the diagram. */
  class CompletionPopup : public QWidget {
   public:
    /** Picking calls `onPick` with the picked label's index in `labels`. */
    CompletionPopup(QStringList labels, std::function<void(qsizetype)> onPick);
    ~CompletionPopup() override;

   protected:
    /** Steers the list from the filter box's keys. */
    bool eventFilter(QObject* watched, QEvent* event) override;

   private:
    /** Moves the selected element by the given `count` of rows. */
    void step(int count);
    /** Picks the given element as the shown one. */
    void pick(const QModelIndex& shown);

    std::function<void(qsizetype)> onPick_;
    std::unique_ptr<Ui::completionPopup> ui_;
    QSortFilterProxyModel* shown_;
  };

  /** Pops a completion popup up at `screenPos` without blocking; it deletes itself once closed. */
  void popUpCompletions(QStringList labels, QPoint screenPos, std::function<void(qsizetype)> onPick);

}  // namespace fluir::editor

#endif
