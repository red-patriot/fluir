#include "editor/app/splash_widget.hpp"

#include <QSvgWidget>

#include "ui_splash_widget.h"

namespace fluir::editor {

  namespace {
    constexpr int kLogoHeightDivisor = 2;  // Logo side is this fraction of the page height.
  }

  SplashWidget::SplashWidget(QWidget* parent) : QWidget(parent), ui_(std::make_unique<Ui::splash>()) {
    ui_->setupUi(this);
    setAttribute(Qt::WA_StyledBackground);  // Subclasses ignore qss backgrounds without it.
    ui_->logo->load(QStringLiteral(":/images/fluir-logo.svg"));
    ui_->newButton->setObjectName("new");  // `new` is a keyword, so uic cannot name the member.
    connect(ui_->newButton, &QPushButton::clicked, this, &SplashWidget::newRequested);
    connect(ui_->open, &QPushButton::clicked, this, &SplashWidget::openRequested);
  }

  SplashWidget::~SplashWidget() = default;

  void SplashWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    const int side = height() / kLogoHeightDivisor;
    ui_->logo->setFixedSize(side, side);
  }

}  // namespace fluir::editor
