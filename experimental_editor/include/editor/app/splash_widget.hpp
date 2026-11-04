#ifndef FLUIR_EDITOR_APP_SPLASH_WIDGET_HPP
#define FLUIR_EDITOR_APP_SPLASH_WIDGET_HPP

#include <memory>
#include <QWidget>

namespace Ui {
  class splash;
}

namespace fluir::editor {

  /** Landing page shown before a module is open. */
  class SplashWidget : public QWidget {
    Q_OBJECT

   public:
    explicit SplashWidget(QWidget* parent = nullptr);
    ~SplashWidget() override;

   signals:
    void newRequested();
    void openRequested();

   protected:
    void resizeEvent(QResizeEvent* event) override;

   private:
    std::unique_ptr<Ui::splash> ui_;
  };

}  // namespace fluir::editor

#endif
