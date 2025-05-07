from pathlib import Path

from editor.models.elements import Program
from editor.repository.fluir_file import FileManager


class ModuleEditor:
    """A Service to edit Fluir modules"""

    def __init__(self) -> None:
        self._repo = FileManager()
        self._module: Program | None = None
        self._path: Path | None = None

    def get_path(self) -> Path | None:
        """Accesses the path of the current module"""
        return self._path

    def get_name(self) -> str:
        """Accesses the name of the current module"""
        return self._path.name if self._path else ""

    def get(self) -> Program | None:
        """Accesses the contents of the current module"""
        return self._module

    def open_file(self, path: Path) -> None:
        """Opens a new file"""
        if self._module:
            self.close()

        self._module = self._repo.parseFile(path)
        self._path = path

    def close(self) -> None:
        """Closes the current module"""
        # TODO: Check if we need to save?
        self._module = None
        self._path = None
