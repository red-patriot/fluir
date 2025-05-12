from pathlib import Path

from editor.models import EditTransaction, Program
from editor.models.edit_errors import BadEdit
from editor.repository.fluir_file import XMLFileManager
from editor.repository.interface.file_manager import FileManager


class ModuleEditor:
    """A Service to edit Fluir modules"""

    def __init__(self, manager: FileManager = XMLFileManager()) -> None:
        self._repo = manager
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

    def open_module(
        self, module: Program, path: Path = Path("/fake/path.fl")
    ) -> None:
        """Open a"""
        if self._module:
            self.close()

        self._module = module
        self._path = path

    def open_file(self, path: Path) -> None:
        """Opens a new file"""
        return self.open_module(self._repo.parseFile(path), path)

    def edit(self, command: EditTransaction) -> None:
        """Applies an edit to the open program"""
        if self._module is None:
            raise BadEdit("Open a program to make edits")

        # TODO: Save the command for undo/redo
        self._module = command.do(self._module)

    def close(self) -> None:
        """Closes the current module"""
        # TODO: Check if we need to save?
        self._module = None
        self._path = None
