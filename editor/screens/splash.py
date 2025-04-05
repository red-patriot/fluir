from PySide6.QtWidgets import (
    QWidget,
    QPushButton,
    QVBoxLayout,
    QFileDialog,
    QMessageBox,
)
from pathlib import Path
from typing import Optional
from screens.editor import EditorWindow


class SplashScreen(QWidget):
    def __init__(self) -> None:
        super().__init__()
        self._editorWindow: Optional[EditorWindow] = None

        self.setWindowTitle("FLUIR")
        self.resize(400, 300)

        self._new_file_button = QPushButton("New File")
        self._open_file_button = QPushButton("Open File")
        layout = QVBoxLayout(self)

        layout.addWidget(self._new_file_button)
        layout.addWidget(self._open_file_button)

        self._new_file_button.clicked.connect(self._new_file)
        self._open_file_button.clicked.connect(self._open_file)

    def _open_editor(self, file: Path) -> None:
        if file.exists():
            self.hide()
            self._editorWindow = EditorWindow(file)
            self._editorWindow.show()
        else:
            QMessageBox.warning(
                self,
                "Open file failed",
                f"The file {file} does not exist.",
            )

    def _new_file(self) -> None:
        filename, _ = QFileDialog.getSaveFileName(
            self, caption="Create a new Fluir file", filter="Fluir (*.fluir)"
        )
        filepath = self._add_extension(Path(filename))
        if not filepath.exists():
            try:
                with open(filepath, "w") as file:
                    file.write("")
                self._open_editor(filepath)
            except Exception as e:
                QMessageBox.warning(
                    self,
                    "Save file failed",
                    f"Failed to save file {filename}\nFailed with error {e}",
                )
        else:
            QMessageBox.warning(
                self,
                "Save file failed",
                f"File {filename} already exists. Use 'Open' button instead to edit a file.",
            )

    def _open_file(self) -> None:
        filename, _ = QFileDialog.getOpenFileName(
            self, caption="Create a new Fluir file", filter="Fluir (*.fluir)"
        )
        filepath = Path(filename)
        self._open_editor(filepath)

    def _add_extension(self, filepath: Path) -> Path:
        if not filepath.suffix:
            return filepath.with_suffix(".fluir")
        return filepath
