#include "editor/app/splash_widget.hpp"

#include "ui_splash_widget.h"

namespace fluir::editor {

  SplashWidget::SplashWidget(QWidget* parent) : QWidget(parent), ui_(std::make_unique<Ui::splash>()) {
    ui_->setupUi(this);
    ui_->newButton->setObjectName("new");  // `new` is a keyword, so uic cannot name the member.
    connect(ui_->newButton, &QPushButton::clicked, this, &SplashWidget::newRequested);
    connect(ui_->open, &QPushButton::clicked, this, &SplashWidget::openRequested);
  }

  SplashWidget::~SplashWidget() = default;

}  // namespace fluir::editor
