#ifndef FLUIR_EDITOR_APP_SPLASH_WIDGET_HPP
#define FLUIR_EDITOR_APP_SPLASH_WIDGET_HPP

#include <QWidget>

namespace fluir::editor {

  /** Landing page shown before a module is open. */
  class SplashWidget : public QWidget {
    Q_OBJECT

   public:
    explicit SplashWidget(QWidget* parent = nullptr);

   signals:
    void newRequested();
    void openRequested();
  };

}  // namespace fluir::editor

#endif
