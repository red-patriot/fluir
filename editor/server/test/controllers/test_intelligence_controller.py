from pathlib import Path
from unittest.mock import MagicMock

from fastapi import FastAPI
from fastapi.testclient import TestClient

from editor.controllers.intelligence_controller import IntelligenceController
from editor.models.lsp.completion import Completion, Kind
from editor.services.intelligence import IntelligenceService


def _make_app(service: IntelligenceService) -> TestClient:
    app = FastAPI()
    controller = IntelligenceController(service)
    controller.register(app)
    return TestClient(app)


def test_forwards_completions_request_on_post() -> None:
    mock_service = MagicMock(spec=IntelligenceService)
    mock_service.get_completions.return_value = [
        Completion(short_name="+ (binary)", kind=Kind.OPERATOR),
    ]

    client = _make_app(mock_service)

    response = client.post(
        "/api/intelligence/",
        json={"block_id": [1, 2], "path": "/fake/path.fl"},
    )

    assert response.status_code == 200
    mock_service.get_completions.assert_called_with(
        [1, 2], Path("/fake/path.fl")
    )


def test_returns_completions_as_json() -> None:
    mock_service = MagicMock(spec=IntelligenceService)
    mock_service.get_completions.return_value = [
        Completion(
            short_name="function",
            kind=Kind.FUNCTION_DEF,
            description="Define a new function here",
        ),
    ]

    client = _make_app(mock_service)

    response = client.post(
        "/api/intelligence/",
        json={"block_id": [], "path": "/fake/path.fl"},
    )

    assert response.status_code == 200
    data = response.json()
    assert len(data) == 1
    assert data[0]["short_name"] == "function"
    assert data[0]["kind"] == "function"
    assert data[0]["description"] == "Define a new function here"


def test_returns_empty_list_when_no_completions() -> None:
    mock_service = MagicMock(spec=IntelligenceService)
    mock_service.get_completions.return_value = []

    client = _make_app(mock_service)

    response = client.post(
        "/api/intelligence/",
        json={"block_id": [1], "path": "/fake/path.fl"},
    )

    assert response.status_code == 200
    assert response.json() == []
