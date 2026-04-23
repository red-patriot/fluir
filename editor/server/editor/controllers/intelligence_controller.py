from pathlib import Path
from typing import override

from fastapi import FastAPI

from editor.controllers.interface.controller import Controller
from editor.models.intelligence_requests import (
    CompletionRequest,
    TypesRequest,
)
from editor.models.lsp.completion import Completion
from editor.services.intelligence import IntelligenceService


class IntelligenceController(Controller):
    """Controller for intelligence requests"""

    def __init__(self, service: IntelligenceService) -> None:
        self._service = service

    @override
    def register(self, app: FastAPI) -> None:
        app.post("/api/intelligence/completions")(self.completions)
        app.post("/api/intelligence/types")(self.types)

    def completions(self, request: CompletionRequest) -> list[Completion]:
        """Handles requests for completions"""
        return self._service.get_completions(
            request.block_id, Path(request.path)
        )

    def types(self, request: TypesRequest) -> list[str]:
        """Handles requests for types"""
        return self._service.get_types(request.block_id, Path(request.path))
