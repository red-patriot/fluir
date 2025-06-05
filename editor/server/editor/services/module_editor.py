from pathlib import Path
from typing import Annotated

from pydantic import Field

from editor.models import Function, Location, Program
from editor.models.edit_errors import BadEdit, EditorError
from editor.repository.fluir_file import XMLFileManager
from editor.repository.interface.file_manager import FileManager
from editor.services.transaction import EditTransaction


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

    def new_module(self) -> None:
        """Create a new module"""
        self._path = None
        # For now, create an empty main when making a new module
        # TODO: Update this in the future
        self._module = Program(
            [
                Function(
                    name="main",
                    location=Location(0, 0, 0, 100, 100),
                    id=1,
                )
            ]
        )

    def open_module(
        self, module: Program, path: Path | None = Path("/fake/path.fl")
    ) -> None:
        """Open a module already parsed"""
        if self._module:
            self.close()

        self._module = module
        self._path = path

    def open_file(self, path: Path) -> None:
        """Opens a new file"""
        return self.open_module(self._repo.parseFile(path), path)

    def save_file(self, path: Path | None = None) -> None:
        """Save the current module to disk"""
        path = path or self._path
        if not self._module or not path:
            raise EditorError("No module or path provided")
        self._repo.writeFile(self._module, path)
        # After saving, update the current path
        self._path = path

    def edit(
        self,
        command: Annotated[
            EditTransaction, Field(discriminator="discriminator")
        ],
    ) -> None:
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
