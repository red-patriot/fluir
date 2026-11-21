#include <QApplication>
#include <QCommandLineParser>
#include <QFontDatabase>

#include "editor/app/main_window.hpp"
#include "editor/app/style.hpp"
#include "editor/core/editor_context.hpp"

namespace fs = std::filesystem;

fluir::editor::MainWindow openWindow(const fluir::editor::EditorContext& ctx, QCommandLineParser& parser) {
  if (parser.positionalArguments().size() > 0) {
    // The (optional) positional arg is a file to directly open, so open it
    auto pathArg = parser.positionalArguments().at(0).toStdString();
    fs::path rawPath{pathArg};
    auto path = std::filesystem::canonical(rawPath);
    return fluir::editor::MainWindow{ctx, std::move(path)};
  }
  return fluir::editor::MainWindow{ctx};
}

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QFontDatabase::addApplicationFont(":/fonts/DejaVuSansMono.ttf");
  app.setStyleSheet(fluir::editor::editorStyle());

  QCommandLineParser argParser;
  argParser.addPositionalArgument("file", "The file to open", "[path]");
  argParser.process(app);
  auto args = argParser.positionalArguments();

  const fluir::editor::EditorContext ctx;
  auto window = openWindow(ctx, argParser);
  window.show();
  return QApplication::exec();
}
