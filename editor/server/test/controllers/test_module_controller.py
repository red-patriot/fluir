from pathlib import Path
from unittest.mock import create_autospec

from fastapi import FastAPI
from fastapi.testclient import TestClient

from editor.controllers.module_controller import ModuleController
from editor.models import Program
from editor.models.module_requests import OpenRequest
from editor.services.module_editor import ModuleEditor


def test_forwards_open_request() -> None:
    mock_editor = create_autospec(ModuleEditor, instance=True)
    expectedPath = "/fake/path/to/module.fl"

    uut = ModuleController(mock_editor)

    uut.open(OpenRequest(path=expectedPath))

    mock_editor.open_file.assert_called_with(Path(expectedPath))


def test_forwards_open_request_on_post() -> None:
    mock_editor = create_autospec(ModuleEditor, instance=True)
    mock_editor.get.return_value = Program()
    expectedPath = "/fake/path/to/module.fl"
    expectedData = '{"declarations":[]}'

    app = FastAPI()

    uut = ModuleController(mock_editor)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post("/api/module/open/", json={"path": expectedPath})

    assert response.status_code == 200
    assert expectedData == response.text
    mock_editor.open_file.assert_called_with(Path(expectedPath))


def test_forwards_close_request() -> None:
    mock_editor = create_autospec(ModuleEditor, instance=True)

    uut = ModuleController(mock_editor)

    uut.close()

    mock_editor.close.assert_called()


def test_forwards_close_request_on_post() -> None:
    mock_editor = create_autospec(ModuleEditor, instance=True)

    app = FastAPI()

    uut = ModuleController(mock_editor)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post("/api/module/close/")

    assert response.status_code == 200
    mock_editor.close.assert_called()
