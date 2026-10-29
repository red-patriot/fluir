#include "editor/app/splash_widget.hpp"

#include <QPushButton>
#include <QVBoxLayout>

namespace fluir::editor {

  SplashWidget::SplashWidget(QWidget* parent) : QWidget(parent) {
    auto* newButton = new QPushButton(tr("New"), this);
    newButton->setObjectName("new");
    auto* openButton = new QPushButton(tr("Open"), this);
    openButton->setObjectName("open");
    connect(newButton, &QPushButton::clicked, this, &SplashWidget::newRequested);
    connect(openButton, &QPushButton::clicked, this, &SplashWidget::openRequested);

    auto* layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(newButton, 0, Qt::AlignHCenter);
    layout->addWidget(openButton, 0, Qt::AlignHCenter);
    layout->addStretch();
  }

}  // namespace fluir::editor
