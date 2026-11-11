#include "editor/app/style.hpp"

#include <QFile>

namespace fluir::editor {

  QString editorStyle() {
    QFile file(":/style/editor.qss");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return {};
    }
    return QString::fromUtf8(file.readAll());
  }

}  // namespace fluir::editor
