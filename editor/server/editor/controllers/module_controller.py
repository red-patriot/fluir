from pathlib import Path
from typing import Annotated, override

from fastapi import Body, FastAPI, HTTPException

from editor.controllers.interface.controller import Controller
from editor.models import Program
from editor.models.module_requests import OpenRequest, SaveRequest
from editor.models.module_responses import ProgramStatus
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import EditTransaction


class ModuleController(Controller):
    """Controller for all module requests"""

    def __init__(self, editor: ModuleEditor) -> None:
        self._editor = editor

    def _make_status(self, saved: bool) -> ProgramStatus:
        program = self._editor.get()
        path = self._editor.get_path()
        if not program:
            raise HTTPException(404, "The requested program does not exist")

        return ProgramStatus(
            saved=saved, path=str(path) if path else None, program=program
        )

    @override
    def register(self, app: FastAPI) -> None:
        app.post("/api/module/new")(self.new)
        app.post("/api/module/open")(self.open)
        app.post("/api/module/close")(self.close)
        app.post("/api/module/edit")(self.edit)
        app.post("/api/module/save")(self.save)

    def new(self) -> ProgramStatus:
        self._editor.new_module()
        program = self._editor.get()
        if not program:
            raise HTTPException(404, "Could not create a new module")
        return self._make_status(saved=False)

    def open(self, request: OpenRequest) -> ProgramStatus:
        """Handles requests to open a module"""
        self._editor.open_file(Path(request.path))
        program = self._editor.get()
        if not program:
            raise HTTPException(404, "The requested program does not exist")
        return self._make_status(saved=True)

    def close(self) -> None:
        """Handles requests to close the current program"""
        self._editor.close()

    def edit(
        self, request: Annotated[EditTransaction, Body()]
    ) -> ProgramStatus:
        self._editor.edit(request)

        return self._make_status(saved=False)

    def save(self, request: SaveRequest) -> ProgramStatus:
        """Handles requests to save the current program"""
        if len(request.path) == 0:
            self._editor.save_file()
        else:
            self._editor.save_file(Path(request.path))

        return self._make_status(saved=True)
