#include <QApplication>
#include <QFontDatabase>

#include "editor/app/main_window.hpp"
#include "editor/app/style.hpp"
#include "editor/core/editor_context.hpp"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QFontDatabase::addApplicationFont(":/fonts/DejaVuSansMono.ttf");
  app.setStyleSheet(fluir::editor::editorStyle());

  const fluir::editor::EditorContext ctx;
  fluir::editor::MainWindow window{ctx};
  window.show();
  return QApplication::exec();
}
