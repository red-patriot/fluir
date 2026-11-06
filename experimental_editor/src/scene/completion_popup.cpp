#include "editor/scene/completion_popup.hpp"

#include <algorithm>
#include <memory>
#include <QKeyEvent>
#include <QStringListModel>
#include <utility>

#include "ui_completion_popup.h"

namespace fluir::editor {
  CompletionPopup::CompletionPopup(QStringList labels, std::function<void(qsizetype)> onPick) :
    QWidget(nullptr, Qt::Popup),
    onPick_(std::move(onPick)),
    ui_(std::make_unique<Ui::completionPopup>()),
    shown_(new QSortFilterProxyModel(this)) {
    ui_->setupUi(this);
    shown_->setSourceModel(new QStringListModel(std::move(labels), shown_));
    shown_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui_->list->setModel(shown_);
    ui_->list->setFocusPolicy(Qt::NoFocus);
    ui_->list->setCurrentIndex(shown_->index(0, 0));
    setFocusProxy(ui_->filter);
    ui_->filter->installEventFilter(this);
    connect(ui_->filter, &QLineEdit::textChanged, this, [this](const QString& text) {
      shown_->setFilterFixedString(text);
      ui_->list->setCurrentIndex(shown_->index(0, 0));
    });
    connect(ui_->list, &QListView::clicked, this, [this](const QModelIndex& index) { pick(index); });
  }

  CompletionPopup::~CompletionPopup() = default;

  bool CompletionPopup::eventFilter(QObject* watched, QEvent* event) {
    if (watched != ui_->filter || event->type() != QEvent::KeyPress) {
      return QWidget::eventFilter(watched, event);
    }
    switch (static_cast<QKeyEvent*>(event)->key()) {
      case Qt::Key_Down:
        step(1);
        return true;
      case Qt::Key_Up:
        step(-1);
        return true;
      case Qt::Key_Return:
      case Qt::Key_Enter:
        if (ui_->list->currentIndex().isValid()) {
          pick(ui_->list->currentIndex());
        }
        return true;
      case Qt::Key_Escape:
        close();
        return true;
      default:
        return QWidget::eventFilter(watched, event);
    }
  }

  void CompletionPopup::step(int by) {
    const int rows = shown_->rowCount();
    if (rows == 0) {
      return;
    }
    ui_->list->setCurrentIndex(shown_->index(std::clamp(ui_->list->currentIndex().row() + by, 0, rows - 1), 0));
  }

  void CompletionPopup::pick(const QModelIndex& shown) {
    const qsizetype index = shown_->mapToSource(shown).row();
    const std::function<void(qsizetype)> onPick = onPick_;
    close();
    onPick(index);
  }

  void popUpCompletions(QStringList labels, QPoint screenPos, std::function<void(qsizetype)> onPick) {
    auto popup = std::make_unique<CompletionPopup>(std::move(labels), std::move(onPick));
    popup->setAttribute(Qt::WA_DeleteOnClose);
    popup->move(screenPos);
    CompletionPopup* shown = popup.release();
    shown->show();
    shown->setFocus();
  }

}  // namespace fluir::editor
