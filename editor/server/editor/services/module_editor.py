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

    def __init__(
        self, manager: FileManager = XMLFileManager(), stack_max: int = 100
    ) -> None:
        self._repo = manager
        self._module: Program | None = None
        self._path: Path | None = None
        self._stack_max = stack_max
        self._undo_stack: list[EditTransaction] = []
        self._redo_stack: list[EditTransaction] = []

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
        self._undo_stack.clear()
        self._redo_stack.clear()

    def open_module(
        self, module: Program, path: Path | None = Path("/fake/path.fl")
    ) -> None:
        """Open a module already parsed"""
        if self._module:
            self.close()

        self._module = module
        self._path = path
        self._undo_stack.clear()
        self._redo_stack.clear()

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
        """Edits the open module with the given transaction and clears the redo stack"""
        self._apply(command)
        self._redo_stack.clear()

    def _apply(self, action: EditTransaction) -> None:
        """Applies an edit to the open program"""
        if self._module is None:
            raise BadEdit("Open a program to make edits")
        self._module = action.do(self._module)
        self._push_undo(action)

    def undo(self) -> None:
        if self._module is None:
            raise BadEdit("Open a program to make edits")

        if not self.can_undo():
            return
        action = self._undo_stack.pop()
        self._module = action.undo(self._module)
        self._push_redo(action)

    def redo(self) -> None:
        if not self.can_redo():
            return
        action = self._redo_stack.pop()
        self._apply(action)

    def can_undo(self) -> bool:
        return len(self._undo_stack) > 0

    def can_redo(self) -> bool:
        return len(self._redo_stack) > 0

    def close(self) -> None:
        """Closes the current module"""
        # TODO: Check if we need to save?
        self._module = None
        self._path = None
        self._undo_stack.clear()
        self._redo_stack.clear()

    def _push_redo(self, action: EditTransaction) -> None:
        """Pushes an action onto the redo stack"""
        self._redo_stack.append(action)
        if len(self._redo_stack) > self._stack_max:
            self._redo_stack.pop(0)

    def _push_undo(self, action: EditTransaction) -> None:
        """Pushes an action onto the undo stack"""
        self._undo_stack.append(action)
        if len(self._undo_stack) > self._stack_max:
            self._undo_stack.pop()
