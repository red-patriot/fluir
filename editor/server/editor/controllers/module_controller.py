from pathlib import Path
from typing import override

from fastapi import FastAPI, HTTPException

from editor.controllers.interface.controller import Controller
from editor.models import Program
from editor.models.module_requests import OpenRequest
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import MoveElement


class ModuleController(Controller):
    """Controller for all module requests"""

    def __init__(self, editor: ModuleEditor) -> None:
        self._editor = editor

    @override
    def register(self, app: FastAPI) -> None:
        app.post("/api/module/open/")(self.open)
        app.post("/api/module/close")(self.close)
        app.post("/api/module/edit")(self.edit)

    def open(self, request: OpenRequest) -> Program:
        """Handles requests to open a module"""
        self._editor.open_file(Path(request.path))
        program = self._editor.get()
        if not program:
            raise HTTPException(404, "The requested program does not exist")
        return program

    def close(self) -> None:
        """Handles requests to close the current"""
        self._editor.close()

    def edit(self, request: MoveElement) -> Program:
        self._editor.edit(request)
        program = self._editor.get()
        if not program:
            raise HTTPException(404, "The requested program does not exist")
        return program
