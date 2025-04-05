from PySide6.QtWidgets import QWidget
from pathlib import Path


class EditorWindow(QWidget):
    def __init__(self, file: Path) -> None:
        super().__init__()
        self._file: Path = file

        self.setWindowTitle("Fluir Editor")
