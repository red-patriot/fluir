#include "editor/scene/completion_popup.hpp"

#include <algorithm>
#include <memory>
#include <QKeyEvent>
#include <QStringListModel>
#include <QVBoxLayout>
#include <utility>

namespace fluir::editor {
  namespace {

    constexpr int WIDTH_PX = 240;
    constexpr int HEIGHT_PX = 260;

  }  // namespace

  CompletionPopup::CompletionPopup(QStringList labels, std::function<void(qsizetype)> onPick) :
    QWidget(nullptr, Qt::Popup),
    onPick_(std::move(onPick)),
    filter_(new QLineEdit(this)),
    list_(new QListView(this)),
    shown_(new QSortFilterProxyModel(this)) {
    shown_->setSourceModel(new QStringListModel(std::move(labels), shown_));
    shown_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    list_->setModel(shown_);
    list_->setFocusPolicy(Qt::NoFocus);
    list_->setCurrentIndex(shown_->index(0, 0));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(filter_);
    layout->addWidget(list_);
    resize(WIDTH_PX, HEIGHT_PX);
    setFocusProxy(filter_);
    filter_->installEventFilter(this);
    connect(filter_, &QLineEdit::textChanged, this, [this](const QString& text) {
      shown_->setFilterFixedString(text);
      list_->setCurrentIndex(shown_->index(0, 0));
    });
    connect(list_, &QListView::clicked, this, [this](const QModelIndex& index) { pick(index); });
  }

  bool CompletionPopup::eventFilter(QObject* watched, QEvent* event) {
    if (watched != filter_ || event->type() != QEvent::KeyPress) {
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
        if (list_->currentIndex().isValid()) {
          pick(list_->currentIndex());
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
    list_->setCurrentIndex(shown_->index(std::clamp(list_->currentIndex().row() + by, 0, rows - 1), 0));
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
